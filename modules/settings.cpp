// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// Settings module for managing Terminstant configuration and dependencies

#include <ncurses.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../menu.hpp"
#include "settings.hpp"
#include "sudo_settings.hpp"

// Forward declarations for ncurses functions
extern void initializeCurses();
extern void setupResizeHandler();

using namespace std;

// Function to check if a program is installed
bool isProgramInstalled(const std::string& program_name) {
    std::string command = "which " + program_name + " >/dev/null 2>&1";
    return system(command.c_str()) == 0;
}

// Function to check if a package is installed (for apt-based systems)
bool isPackageInstalled(const std::string& package_name) {
    std::string command = "dpkg -l | grep -q '^ii.*" + package_name + "'";
    return system(command.c_str()) == 0;
}

// Function to install a package using apt
bool installPackage(const std::string& package_name) {
    std::string sudo_cmd = getSudoCommand();
    std::string command;
    
    // Temporarily exit ncurses to handle password prompts properly
    endwin();
    
    if (sudo_cmd == "su") {
        // For su, we need to use su -c "command" format
        command = "su -c \"apt update && apt install -y " + package_name + "\"";
    } else {
        // For sudo, we can use the original format
        command = sudo_cmd + " apt update && " + sudo_cmd + " apt install -y " + package_name;
    }
    
    int result = system(command.c_str());
    
    // Reinitialize ncurses after the command
    initializeCurses();
    setupResizeHandler();
    
    return result == 0;
}

// Function to get list of required programs
std::vector<ProgramInfo> getRequiredPrograms() {
    std::vector<ProgramInfo> programs = {
        {"htop", "htop", "Interactive process viewer", false, "sudo apt install -y htop"},
        {"screenfetch", "screenfetch", "System information display", false, "sudo apt install -y screenfetch"},
        {"lsblk", "util-linux", "Block device information", false, "sudo apt install -y util-linux"},
        {"tar", "tar", "Archive utility", false, "sudo apt install -y tar"},
        {"curl", "curl", "Data transfer utility", false, "sudo apt install -y curl"},
        {"wget", "wget", "Web download utility", false, "sudo apt install -y wget"},
        {"nano", "nano", "Text editor", false, "sudo apt install -y nano"},
        {"vim", "vim", "Advanced text editor", false, "sudo apt install -y vim"},
        {"git", "git", "Version control system", false, "sudo apt install -y git"},
        {"tree", "tree", "Directory listing utility", false, "sudo apt install -y tree"},
        {"xinput", "xinput", "Input Device Configurator", false, "sudo apt install -y xinput"},
        {"iftop", "iftop", "Network Traffic Monitor", false, "sudo apt install -y iftop"}
    };
    
    // Check which programs are installed
    for (auto& program : programs) {
        program.is_installed = isProgramInstalled(program.name);
    }
    
    return programs;
}

void showSettingsMenu() {
    std::vector<std::string> options = {
        "Programs",
        "Back"
    };
    
    Menu menu("=== Terminstant Settings ===", options);
    
    int choice;
    while ((choice = menu.run()) != -1) {
        switch (choice) {
            case 0:
                showProgramsMenu();
                break;
            case 1:
                return;
        }
    }
}

void showProgramsMenu() {
    std::vector<std::string> options = {
        "Check and Install Required Programs",
        "View Installed Programs",
        "Back"
    };
    
    Menu menu("=== Programs ===", options);
    
    int choice;
    while ((choice = menu.run()) != -1) {
        switch (choice) {
            case 0:
                showDependencyChecker();
                break;
            case 1:
                showInstalledPrograms();
                break;
            case 2:
                return;
        }
    }
}

void showDependencyChecker() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Checking Required Programs ===");
    mvprintw(2, 0, "Scanning system for required programs...");
    refresh();
    
    std::vector<ProgramInfo> programs = getRequiredPrograms();
    
    clear();
    mvprintw(0, 0, "=== Required Programs Status ===");
    
    int line = 2;
    int installed_count = 0;
    int total_count = programs.size();
    
    for (const auto& program : programs) {
        std::string status = program.is_installed ? "[INSTALLED]" : "[MISSING]";
        std::string color_code = program.is_installed ? "\033[32m" : "\033[31m"; // Green for installed, Red for missing
        
        mvprintw(line, 0, "%s: %s", program.name.c_str(), status.c_str());
        
        if (program.is_installed) {
            installed_count++;
        }
        
        line++;
        
        if (line >= maxLines - 4) break; // Leave space for summary and controls
    }
    
    // Summary
    mvprintw(maxLines - 4, 0, "=== Summary ===");
    mvprintw(maxLines - 3, 0, "Installed: %d/%d programs", installed_count, total_count);
    
    if (installed_count < total_count) {
        mvprintw(maxLines - 2, 0, "Some programs are missing. Would you like to install them?");
        mvprintw(maxLines - 1, 0, "Press 'i' to install missing programs, any other key to continue...");
    } else {
        mvprintw(maxLines - 2, 0, "All required programs are installed!");
        mvprintw(maxLines - 1, 0, "Press any key to continue...");
    }
    
    refresh();
    
    int ch = getch();
    if (ch == 'i' || ch == 'I') {
        installMissingPrograms(programs);
    }
}

void showInstalledPrograms() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    std::vector<ProgramInfo> programs = getRequiredPrograms();
    
    mvprintw(0, 0, "=== Installed Programs ===");
    
    int line = 2;
    for (const auto& program : programs) {
        if (program.is_installed) {
            mvprintw(line, 0, "[OK] %s", program.name.c_str());
            line++;
        }
        
        if (line >= maxLines - 2) break;
    }
    
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

void installMissingPrograms(const std::vector<ProgramInfo>& programs) {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Installing Missing Programs ===");
    mvprintw(2, 0, "This will require sudo privileges.");
    mvprintw(3, 0, "Press 'y' to continue or any other key to cancel...");
    refresh();
    
    int ch = getch();
    if (ch != 'y' && ch != 'Y') {
        return;
    }
    
    clear();
    mvprintw(0, 0, "=== Installing Missing Programs ===");
    
    int line = 2;
    int success_count = 0;
    int total_missing = 0;
    
    for (const auto& program : programs) {
        if (!program.is_installed) {
            total_missing++;
            mvprintw(line, 0, "Installing %s...", program.name.c_str());
            refresh();
            
            bool success = installPackage(program.package_name);
            if (success) {
                mvprintw(line, 0, "[OK] %s installed successfully", program.name.c_str());
                success_count++;
            } else {
                mvprintw(line, 0, "[ERROR] Failed to install %s", program.name.c_str());
            }
            
            line++;
            if (line >= maxLines - 3) break;
        }
    }
    
    mvprintw(maxLines - 3, 0, "=== Installation Summary ===");
    mvprintw(maxLines - 2, 0, "Successfully installed: %d/%d programs", success_count, total_missing);
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}
