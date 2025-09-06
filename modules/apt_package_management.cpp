// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#include <ncurses.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstdlib>
#include "../menu.hpp"
#include "sudo_settings.hpp"

// Helper function to get package names from user
std::string getPackageNamesFromUser(const std::string& operation) {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== %s ===", operation.c_str());
    mvprintw(2, 0, "Enter package names separated by spaces");
    mvprintw(3, 0, "Example: package1 package2 package3");
    mvprintw(4, 0, "");
    mvprintw(5, 0, "Package names: ");
    refresh();
    
    std::string packageNames = "";
    int ch;
    int cursorX = 15; // Starting cursor position
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return ""; // Return empty string to indicate cancellation
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!packageNames.empty()) {
                packageNames.pop_back();
                cursorX--;
                // Clear the character at current position
                mvprintw(5, cursorX, " ");
                move(5, cursorX);
                refresh();
            }
            continue;
        }
        
        // Accept alphanumeric characters, spaces, hyphens, underscores, and dots
        if (isalnum(ch) || ch == ' ' || ch == '-' || ch == '_' || ch == '.') {
            packageNames += (char)ch;
            mvprintw(5, cursorX, "%c", ch);
            cursorX++;
            move(5, cursorX);
            refresh();
        }
    }
    
    return packageNames;
}

void showAptPackageManagementMenu() {
    std::vector<std::string> options = {
        "Install Packages",
        "Purge Packages",
        "Remove Packages",
        "Search in Package Description",
        "Show Package Details",
        "Update Repositories",
        "Upgrade System",
        "Back"
    };
    Menu menu("=== Package Management via APT ===", options);
    
    // Function mapping for APT package management menu
    std::map<std::string, std::function<void()>> aptMenuFunctions = {
        {"Install Packages", []() { 
            std::string packageNames = getPackageNamesFromUser("Install Packages");
            if (packageNames.empty()) {
                return; // User cancelled
            }
            
            endwin(); // End ncurses before executing command
            std::string sudo_cmd = getSudoCommand();
            std::string command;
            if (sudo_cmd == "su") {
                command = "su -c \"apt install " + packageNames + "\"";
            } else {
                command = sudo_cmd + " apt install " + packageNames;
            }
            system(command.c_str());
            initializeCurses(); // Reinitialize ncurses after returning
        }},
        {"Purge Packages", []() { 
            std::string packageNames = getPackageNamesFromUser("Purge Packages");
            if (packageNames.empty()) {
                return; // User cancelled
            }
            
            endwin();
            std::string sudo_cmd = getSudoCommand();
            std::string command;
            if (sudo_cmd == "su") {
                command = "su -c \"apt purge " + packageNames + "\"";
            } else {
                command = sudo_cmd + " apt purge " + packageNames;
            }
            system(command.c_str());
            initializeCurses();
        }},
        {"Remove Packages", []() { 
            std::string packageNames = getPackageNamesFromUser("Remove Packages");
            if (packageNames.empty()) {
                return; // User cancelled
            }
            
            endwin();
            std::string sudo_cmd = getSudoCommand();
            std::string command;
            if (sudo_cmd == "su") {
                command = "su -c \"apt remove " + packageNames + "\"";
            } else {
                command = sudo_cmd + " apt remove " + packageNames;
            }
            system(command.c_str());
            initializeCurses();
        }},
        {"Search in Package Description", []() { 
            std::string packageNames = getPackageNamesFromUser("Search Packages");
            if (packageNames.empty()) {
                return; // User cancelled
            }
            
            // Execute apt search and capture output within ncurses
            clear();
            refresh();
            
            int maxLines, maxCols;
            getmaxyx(stdscr, maxLines, maxCols);
            
            mvprintw(0, 0, "=== Search Results for: %s ===", packageNames.c_str());
            mvprintw(2, 0, "Searching packages...");
            refresh();
            
            // Execute apt search command and capture output
            std::string command = "apt search " + packageNames;
            FILE* pipe = popen(command.c_str(), "r");
            if (pipe) {
                char buffer[1024];
                int line = 3;
                
                while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 3) {
                    std::string text = buffer;
                    // Remove trailing newline
                    if (!text.empty() && text.back() == '\n') text.pop_back();
                    
                    // Truncate to fit screen width
                    if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
                    
                    mvprintw(line, 0, "%s", text.c_str());
                    line++;
                }
                pclose(pipe);
            }
            
            mvprintw(maxLines - 2, 0, "Press any key to continue...");
            refresh();
            getch();
        }},
        {"Show Package Details", []() { 
            std::string packageNames = getPackageNamesFromUser("Show Package Details");
            if (packageNames.empty()) {
                return; // User cancelled
            }
            
            // Execute apt show and capture output within ncurses
            clear();
            refresh();
            
            int maxLines, maxCols;
            getmaxyx(stdscr, maxLines, maxCols);
            
            mvprintw(0, 0, "=== Package Details for: %s ===", packageNames.c_str());
            mvprintw(2, 0, "Retrieving package information...");
            refresh();
            
            // Execute apt show command and capture output
            std::string command = "apt show " + packageNames;
            FILE* pipe = popen(command.c_str(), "r");
            if (pipe) {
                char buffer[1024];
                int line = 3;
                
                while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 3) {
                    std::string text = buffer;
                    // Remove trailing newline
                    if (!text.empty() && text.back() == '\n') text.pop_back();
                    
                    // Truncate to fit screen width
                    if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
                    
                    mvprintw(line, 0, "%s", text.c_str());
                    line++;
                }
                pclose(pipe);
            }
            
            mvprintw(maxLines - 2, 0, "Press any key to continue...");
            refresh();
            getch();
        }},
        {"Update Repositories", []() { 
            // Update repositories doesn't need package names
            endwin();
            std::string sudo_cmd = getSudoCommand();
            std::string command;
            if (sudo_cmd == "su") {
                command = "su -c \"apt update\"";
            } else {
                command = sudo_cmd + " apt update";
            }
            system(command.c_str());
            initializeCurses();
        }},
        {"Upgrade System", []() { 
            // Upgrade system doesn't need package names
            endwin();
            std::string sudo_cmd = getSudoCommand();
            std::string command;
            if (sudo_cmd == "su") {
                command = "su -c \"apt upgrade\"";
            } else {
                command = sudo_cmd + " apt upgrade";
            }
            system(command.c_str());
            initializeCurses();
        }}
    };
    
    menu.executeMenu(aptMenuFunctions);
}
