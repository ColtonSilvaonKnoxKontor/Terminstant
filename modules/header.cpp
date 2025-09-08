// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// header module for customizing the header text displayed in menus

#include <ncursesw/ncurses.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <map>
#include <functional>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <errno.h>
#include "../menu.hpp"

using namespace std;

// Configuration file constants
static const std::string CONFIG_DIR_NAME = ".terminstant";
static const std::string HEADER_CONFIG_FILE = "header.conf";

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

// Function to get header config file path
static std::string getHeaderConfigPath() {
    return getConfigDirPath() + "/" + HEADER_CONFIG_FILE;
}

// Forward declaration
static bool saveHeaderConfig(const std::string& type, const std::string& text, int colorPair = 2);

// Function to load header configuration
static void loadHeaderConfig() {
    std::string configPath = getHeaderConfigPath();
    std::ifstream configFile(configPath);
    
    if (!configFile.is_open()) {
        // No config file exists, set default and save it
        std::string defaultHeader = "Terminstant v1.0 (2025 SILVASYSTEMS by Colton Silva)";
        Menu::setGlobalHeader(defaultHeader, 2);
        saveHeaderConfig("default", defaultHeader, 2);
        return;
    }
    
    std::string line;
    std::string headerType = "";
    std::string headerText = "";
    int colorPair = 2; // Default color pair
    
    while (std::getline(configFile, line)) {
        if (line.find("type=") == 0) {
            headerType = line.substr(5);
        } else if (line.find("text=") == 0) {
            headerText = line.substr(5);
        } else if (line.find("color=") == 0) {
            colorPair = std::stoi(line.substr(6));
        }
    }
    configFile.close();
    
    // Apply the loaded configuration
    if (!headerText.empty()) {
        Menu::setGlobalHeader(headerText, colorPair);
    } else {
        // Config file exists but has no text, use default
        std::string defaultHeader = "Terminstant v1.0 (2025 SILVASYSTEMS by Colton Silva)";
        Menu::setGlobalHeader(defaultHeader, 2);
    }
}

// Function to save header configuration
static bool saveHeaderConfig(const std::string& type, const std::string& text, int colorPair) {
    if (!ensureConfigDirExists()) {
        return false;
    }
    
    std::string configPath = getHeaderConfigPath();
    std::ofstream configFile(configPath);
    
    if (!configFile.is_open()) {
        return false;
    }
    
    configFile << "# Terminstant Header Configuration\n";
    configFile << "# Generated automatically - do not edit manually\n";
    configFile << "type=" << type << "\n";
    configFile << "text=" << text << "\n";
    configFile << "color=" << colorPair << "\n";
    
    configFile.close();
    return true;
}

// Function to get distribution information
static std::string getDistributionInfo() {
    std::string distro_info = "";
    
    // Try to read from /etc/os-release first
    std::ifstream os_release("/etc/os-release");
    std::string name = "";
    std::string version_codename = "";
    
    if (os_release.is_open()) {
        std::string line;
        while (std::getline(os_release, line)) {
            if (line.find("NAME=") == 0) {
                name = line.substr(5);
                // Remove quotes if present
                if (name.front() == '"' && name.back() == '"') {
                    name = name.substr(1, name.length() - 2);
                }
            } else if (line.find("VERSION_CODENAME=") == 0) {
                version_codename = line.substr(17);
                // Remove quotes if present
                if (version_codename.front() == '"' && version_codename.back() == '"') {
                    version_codename = version_codename.substr(1, version_codename.length() - 2);
                }
            }
        }
        os_release.close();
        
        if (!name.empty()) {
            distro_info = name;
            if (!version_codename.empty()) {
                distro_info += " (" + version_codename + ")";
            }
        }
    }
    
    // Fallback to lsb_release if /etc/os-release didn't work
    if (distro_info.empty()) {
        FILE* pipe = popen("lsb_release -d -c 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            std::string description = "";
            std::string codename = "";
            
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                std::string line(buffer);
                if (line.find("Description:") != std::string::npos) {
                    size_t tab_pos = line.find('\t');
                    if (tab_pos != std::string::npos) {
                        description = line.substr(tab_pos + 1);
                        // Remove newline
                        if (!description.empty() && description.back() == '\n') {
                            description.pop_back();
                        }
                    }
                } else if (line.find("Codename:") != std::string::npos) {
                    size_t tab_pos = line.find('\t');
                    if (tab_pos != std::string::npos) {
                        codename = line.substr(tab_pos + 1);
                        // Remove newline
                        if (!codename.empty() && codename.back() == '\n') {
                            codename.pop_back();
                        }
                    }
                }
            }
            pclose(pipe);
            
            if (!description.empty()) {
                distro_info = description;
                if (!codename.empty()) {
                    distro_info += " (" + codename + ")";
                }
            }
        }
    }
    
    // Final fallback
    if (distro_info.empty()) {
        distro_info = "Linux Distribution";
    }
    
    return distro_info;
}

// Function to get user input for custom header
static std::string getCustomHeaderInput() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(3, 0, "Enter your custom header text (max 80 characters):");
    mvprintw(4, 0, "This will be displayed at the top of all menus, centered.");
    mvprintw(5, 0, "(Press ESC to cancel)");
    mvprintw(7, 0, "> ");
    
    // Enable echo and cursor for input
    echo();
    curs_set(1);
    
    char input[81]; // 80 characters + null terminator
    int ch;
    int pos = 0;
    
    while ((ch = getch()) != 27 && ch != 10 && pos < 80) { // ESC or Enter, max 80 chars
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                mvprintw(7, 2 + pos, " ");
                move(7, 2 + pos);
            }
        } else if (ch >= 32 && ch <= 126) { // Printable characters
            input[pos] = ch;
            mvaddch(7, 2 + pos, ch);
            pos++;
        }
        refresh();
    }
    
    input[pos] = '\0';
    
    // Disable echo and cursor
    noecho();
    curs_set(0);
    
    if (ch == 27) {
        return ""; // Cancelled
    }
    
    return std::string(input);
}

// Function to set default header
static void setDefaultHeader() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    std::string defaultHeader = "Terminstant v1.0 (2025 SILVASYSTEMS by Colton Silva)";
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    // Show what will be set
    if (has_colors()) attron(COLOR_PAIR(2)); // Color pair 2
    mvprintw(3, 0, "Setting header to default:");
    mvprintw(4, 0, "%s", defaultHeader.c_str());
    if (has_colors()) attroff(COLOR_PAIR(2));
    
    // Set the global header
    Menu::setGlobalHeader(defaultHeader, 2);
    
    // Save configuration
    bool saved = saveHeaderConfig("default", defaultHeader, 2);
    
    if (has_colors()) attron(COLOR_PAIR(5)); // Green
    if (saved) {
        mvprintw(6, 0, "Default header has been set and saved successfully!");
    } else {
        mvprintw(6, 0, "Default header has been set (warning: could not save configuration)!");
    }
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

// Function to set distribution header
static void setDistributionHeader() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(3, 0, "Detecting distribution information...");
    refresh();
    
    std::string distroHeader = getDistributionInfo();
    
    // Show what will be set
    if (has_colors()) attron(COLOR_PAIR(2)); // Color pair 2
    mvprintw(5, 0, "Setting header to distribution name:");
    mvprintw(6, 0, "%s", distroHeader.c_str());
    if (has_colors()) attroff(COLOR_PAIR(2));
    
    // Set the global header
    Menu::setGlobalHeader(distroHeader, 2);
    
    // Save configuration
    bool saved = saveHeaderConfig("distribution", distroHeader, 2);
    
    if (has_colors()) attron(COLOR_PAIR(5)); // Green
    if (saved) {
        mvprintw(8, 0, "Distribution header has been set and saved successfully!");
    } else {
        mvprintw(8, 0, "Distribution header has been set (warning: could not save configuration)!");
    }
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

// Function to set custom header
static void setCustomHeader() {
    std::string customHeader = getCustomHeaderInput();
    
    if (customHeader.empty()) {
        return; // User cancelled
    }
    
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    // Show what will be set
    if (has_colors()) attron(COLOR_PAIR(2)); // Color pair 2
    mvprintw(3, 0, "Setting header to custom text:");
    mvprintw(4, 0, "%s", customHeader.c_str());
    if (has_colors()) attroff(COLOR_PAIR(2));
    
    // Set the global header
    Menu::setGlobalHeader(customHeader, 2);
    
    // Save configuration
    bool saved = saveHeaderConfig("custom", customHeader, 2);
    
    if (has_colors()) attron(COLOR_PAIR(5)); // Green
    if (saved) {
        mvprintw(6, 0, "Custom header has been set and saved successfully!");
    } else {
        mvprintw(6, 0, "Custom header has been set (warning: could not save configuration)!");
    }
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

void showHeaderMenu() {
    vector<string> options = {
        "Default Header",
        "Distribution Name", 
        "Specify your Own",
        "Back"
    };
    Menu menu("=== Change Header Text ===", options);
    
    // Function mapping for header menu
    std::map<std::string, std::function<void()>> headerFunctions = {
        {"Default Header", setDefaultHeader},
        {"Distribution Name", setDistributionHeader},
        {"Specify your Own", setCustomHeader}
    };
    
    menu.executeMenu(headerFunctions);
}

// Function to initialize header configuration at startup
void initializeHeaderConfig() {
    loadHeaderConfig();
}
