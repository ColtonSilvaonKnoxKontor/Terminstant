// 2025 Colton Silva
// Terminstant - Network Monitor (iftop) submenu

#include <ncurses.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstdio>
#include <algorithm>
#include "../menu.hpp"
#include "network_monitor.hpp"
#include "sudo_settings.hpp"

using namespace std;

static void runIftopCommand(const char* title, const char* cmd) {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    mvprintw(0, 0, "%s", title);
    mvprintw(2, 0, "Executing command in root mode...");
    refresh();
    
    // Temporarily exit ncurses to run iftop
    endwin();
    
    // Execute iftop command with sudo/su
    std::string sudo_cmd = getSudoCommand();
    std::string full_command;
    
    if (sudo_cmd == "su") {
        full_command = "su -c \"" + std::string(cmd) + "\"";
    } else {
        full_command = sudo_cmd + " " + std::string(cmd);
    }
    
    system(full_command.c_str());
    
    // Reinitialize ncurses
    initializeCurses();
    setupResizeHandler();
    
    // Show completion message
    clear();
    mvprintw(0, 0, "=== Command Completed ===");
    mvprintw(2, 0, "The iftop command has been executed.");
    mvprintw(3, 0, "");
    mvprintw(4, 0, "Press any key to continue...");
    refresh();
    getch();
}

static void showAvailableInterfaces() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Available Network Interfaces ===");
    
    // Get available interfaces using ip link show
    FILE* pipe = popen("ip link show | grep -E '^[0-9]+:' | awk '{print $2}' | sed 's/://'", "r");
    if (pipe) {
        char buffer[256];
        int line = 2;
        while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 4) {
            std::string text = buffer;
            if (!text.empty() && text.back() == '\n') text.pop_back();
            if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
            mvprintw(line++, 0, "%s", text.c_str());
        }
        pclose(pipe);
    }
    
    mvprintw(maxLines - 2, 0, "Press any key to continue...");
    refresh();
    getch();
}

static void runIftopOnInterface() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== iftop on Specific Interface ===");
    mvprintw(2, 0, "Available interfaces:");
    
    // Show available interfaces
    FILE* pipe = popen("ip link show | grep -E '^[0-9]+:' | awk '{print $2}' | sed 's/://'", "r");
    if (pipe) {
        char buffer[256];
        int line = 3;
        while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 6) {
            std::string text = buffer;
            if (!text.empty() && text.back() == '\n') text.pop_back();
            if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
            mvprintw(line++, 0, "  %s", text.c_str());
        }
        pclose(pipe);
    }
    
    mvprintw(maxLines - 4, 0, "Enter interface name (or press ESC to go back):");
    mvprintw(maxLines - 3, 0, "> ");
    refresh();
    
    // Get user input for interface name
    std::string interfaceName = "";
    int ch;
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!interfaceName.empty()) {
                interfaceName.pop_back();
                mvprintw(maxLines - 3, 2 + interfaceName.length(), " "); // Clear the character
                move(maxLines - 3, 2 + interfaceName.length()); // Move cursor back
                refresh();
            }
            continue;
        }
        
        // Accept alphanumeric characters and common interface characters
        if (isalnum(ch) || ch == '-' || ch == '_' || ch == '.') {
            interfaceName += (char)ch;
            mvprintw(maxLines - 3, 2 + interfaceName.length() - 1, "%c", ch);
            refresh();
        }
    }
    
    // Validate interface name input
    if (interfaceName.empty()) {
        clear();
        mvprintw(0, 0, "=== No Input ===");
        mvprintw(2, 0, "No interface name entered.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Show confirmation
    clear();
    mvprintw(0, 0, "=== Confirm iftop on Interface ===");
    mvprintw(2, 0, "Interface: %s", interfaceName.c_str());
    mvprintw(3, 0, "Command: iftop -i %s", interfaceName.c_str());
    mvprintw(4, 0, "");
    mvprintw(5, 0, "Are you sure you want to run iftop on this interface?");
    mvprintw(6, 0, "Press 'y' to confirm or any other key to cancel...");
    refresh();
    
    ch = getch();
    if (ch == 'y' || ch == 'Y') {
        std::string cmd = "iftop -i " + interfaceName;
        runIftopCommand("=== iftop on Interface ===", cmd.c_str());
    }
}

void showNetworkMonitorMenu() {
    std::vector<std::string> options = {
        "Run iftop",
        "Run iftop (in Bytes)",
        "Run iftop in Text Mode",
        "Run iftop in Specified Interface",
        "Show Available Interfaces",
        "Back"
    };
    Menu menu("=== Network Monitor (iftop) ===", options);

    // Function map; Menu auto-sorts, mapping uses option string keys
    std::map<std::string, std::function<void()>> fn = {
        {"Run iftop", [](){ runIftopCommand("=== iftop ===", "iftop"); }},
        {"Run iftop (in Bytes)", [](){ runIftopCommand("=== iftop -B ===", "iftop -B"); }},
        {"Run iftop in Text Mode", [](){ runIftopCommand("=== iftop -t ===", "iftop -t"); }},
        {"Run iftop in Specified Interface", runIftopOnInterface},
        {"Show Available Interfaces", showAvailableInterfaces}
    };

    menu.executeMenu(fn);
}
