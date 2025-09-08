// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// Sudo settings module for managing sudo/su preferences

#include <ncurses.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include "../menu.hpp"
#include "sudo_settings.hpp"

using namespace std;

// Global variable to track current sudo method (default to sudo)
std::string sudo_method = "sudo -i";

// Configuration constants
static const std::string CONFIG_DIR_NAME = ".terminstant";
static const std::string SUDO_CONFIG_FILE = "terminstant_sudo.conf";

// Function to get terminstant config directory path
static std::string getConfigDirPath() {
    char* home = getenv("HOME");
    if (home == nullptr) {
        return "/tmp/" + CONFIG_DIR_NAME; // Fallback
    }
    return std::string(home) + "/" + CONFIG_DIR_NAME;
}

// Function to ensure config directory exists
static bool ensureConfigDirExists() {
    std::string configDir = getConfigDirPath();
    
    // Check if directory exists
    struct stat st;
    if (stat(configDir.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true; // Directory already exists
        } else {
            // Path exists but is not a directory
            return false;
        }
    }
    
    // Create directory with mode 0755
    if (mkdir(configDir.c_str(), 0755) == 0) {
        return true;
    }
    
    return false; // Failed to create directory
}

// Function to get sudo config file path
static std::string getSudoConfigPath() {
    return getConfigDirPath() + "/" + SUDO_CONFIG_FILE;
}

void showSudoSettingsMenu() {
    std::vector<std::string> options = {
        "Change Sudo to su",
        "Change su to Sudo",
        "View Current Setting",
        "Back"
    };
    
    Menu menu("=== Sudo Settings ===", options);
    
    // Create function mapping for sudo settings menu
    std::map<std::string, std::function<void()>> sudoSettingsFunctions = {
        {"Change Sudo to su", changeToSu},
        {"Change su to Sudo", changeToSudo},
        {"View Current Setting", showCurrentSetting}
    };
    
    menu.executeMenu(sudoSettingsFunctions);
}

void changeToSu() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Change to su ===");
    mvprintw(2, 0, "This will change the preferred administrative method");
    mvprintw(3, 0, "from 'sudo -i' to 'su' for Terminstant sessions.");
    mvprintw(4, 0, "");
    mvprintw(5, 0, "This is useful when 'sudo' is not available or");
    mvprintw(6, 0, "when you prefer using 'su' for root access.");
    mvprintw(7, 0, "");
    mvprintw(8, 0, "Current setting: %s", sudo_method.c_str());
    mvprintw(9, 0, "New setting: su");
    mvprintw(10, 0, "");
    mvprintw(11, 0, "Press 'y' to confirm or any other key to cancel...");
    
    refresh();
    
    int ch = getch();
    if (ch == 'y' || ch == 'Y') {
        sudo_method = "su";
        saveSudoSetting();
        
        clear();
        mvprintw(0, 0, "=== Setting Changed ===");
        mvprintw(2, 0, "Administrative method changed to 'su'");
        mvprintw(3, 0, "This setting will be used for all Terminstant");
        mvprintw(4, 0, "administrative operations.");
        mvprintw(5, 0, "");
        mvprintw(6, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}

void changeToSudo() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Change to Sudo ===");
    mvprintw(2, 0, "This will change the preferred administrative method");
    mvprintw(3, 0, "from 'su' to 'sudo -i' for Terminstant sessions.");
    mvprintw(4, 0, "");
    mvprintw(5, 0, "This is useful when you have sudo privileges");
    mvprintw(6, 0, "and prefer using 'sudo' for administrative tasks.");
    mvprintw(7, 0, "");
    mvprintw(8, 0, "Current setting: %s", sudo_method.c_str());
    mvprintw(9, 0, "New setting: sudo -i");
    mvprintw(10, 0, "");
    mvprintw(11, 0, "Press 'y' to confirm or any other key to cancel...");
    
    refresh();
    
    int ch = getch();
    if (ch == 'y' || ch == 'Y') {
        sudo_method = "sudo -i";
        saveSudoSetting();
        
        clear();
        mvprintw(0, 0, "=== Setting Changed ===");
        mvprintw(2, 0, "Administrative method changed to 'sudo -i'");
        mvprintw(3, 0, "This setting will be used for all Terminstant");
        mvprintw(4, 0, "administrative operations.");
        mvprintw(5, 0, "");
        mvprintw(6, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}

void showCurrentSetting() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Current Sudo Setting ===");
    mvprintw(2, 0, "Current administrative method: %s", sudo_method.c_str());
    mvprintw(3, 0, "");
    
    if (sudo_method == "su") {
        mvprintw(4, 0, "Using 'su' for administrative operations.");
        mvprintw(5, 0, "This will prompt for root password.");
        mvprintw(6, 0, "Useful when sudo is not available or configured.");
    } else {
        mvprintw(4, 0, "Using 'sudo -i' for administrative operations.");
        mvprintw(5, 0, "This will prompt for user password.");
        mvprintw(6, 0, "Requires sudo privileges to be configured.");
    }
    
    mvprintw(8, 0, "Press any key to continue...");
    refresh();
    getch();
}

void saveSudoSetting() {
    // Ensure config directory exists
    if (!ensureConfigDirExists()) {
        return; // Cannot create config directory
    }
    
    // Save the setting to a configuration file in the config directory
    std::string configPath = getSudoConfigPath();
    std::ofstream config_file(configPath);
    if (config_file.is_open()) {
        config_file << "sudo_method=" << sudo_method << std::endl;
        config_file.close();
    }
}

void loadSudoSetting() {
    std::string newConfigPath = getSudoConfigPath();
    std::string oldConfigPath = "terminstant_sudo.conf";
    
    // Try to load from new location first
    std::ifstream config_file(newConfigPath);
    if (config_file.is_open()) {
        std::string line;
        if (std::getline(config_file, line)) {
            if (line.find("sudo_method=") == 0) {
                sudo_method = line.substr(12); // Remove "sudo_method=" prefix
            }
        }
        config_file.close();
        return;
    }
    
    // If new location doesn't exist, try old location for migration
    config_file.open(oldConfigPath);
    if (config_file.is_open()) {
        std::string line;
        if (std::getline(config_file, line)) {
            if (line.find("sudo_method=") == 0) {
                sudo_method = line.substr(12); // Remove "sudo_method=" prefix
            }
        }
        config_file.close();
        
        // Migrate to new location
        saveSudoSetting();
        
        // Remove old config file
        std::remove(oldConfigPath.c_str());
    }
}

// Function to get the current sudo command for use in other modules
std::string getSudoCommand() {
    return sudo_method;
}
