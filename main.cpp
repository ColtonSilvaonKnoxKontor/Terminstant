// 2025 Colton Silva
//
// Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// Dependency Requirements:
// - libncurses5-dev
// - libncursesw5-dev
//
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>
#include "modules/htop.hpp"
#include "modules/taskkiller.hpp"
#include "modules/screenfetch.hpp"
#include "modules/lsblk.hpp"
#include "modules/settings.hpp"
#include "modules/sudo_settings.hpp"
#include "modules/input_devices.hpp"
#include "modules/uname_info.hpp"
#include "modules/network_monitor.hpp"
#include "modules/apt_package_management.hpp"
#include "modules/ls.hpp"
#include "modules/header.hpp"
#include "modules/term_menu.hpp"
#include "menu.hpp"

using namespace std;
// Forward declarations of submenus
void showHtopMenu();
void showIllegalOperationMenu();
void showAdministrationMenu();
void showAptPackageManagementMenu();
void showSystemSettingMenu();
void showSystemInfoMenu();
void showUtilitiesMenu();
void showInternetMenu();
void showSettingsMenu();
void showSudoSettingsMenu();
void showInputDevicesMenu();
void showNetworkMonitorMenu();
void showCustomizeMenu();
void showCustomUserMenus(const std::string& parentMenu);

int main() {
    initializeCurses();
    setupResizeHandler();
    loadSudoSetting();
    initializeHomeDirectory();
    initializeHeaderConfig();
    initializeCustomMenus();

    std::vector<std::string> mainOptions = {
        "Illegal Operation",
        "Administration",
        "System Setting",
        "System Information",
        "Utilities",
        "Internet",
        "Customize",
        "Terminstant Settings",
        "Exit"
    };

    Menu::setGlobalHelp("Up/Down: Navigate  Enter: Select  ESC: Back/Exit", 1);
    // Header is now set by initializeHeaderConfig() - no hardcoded override
    Menu mainMenu("=== Main Menu ===", mainOptions);

    // Function mapping for main menu
    std::map<std::string, std::function<void()>> mainMenuFunctions = {
        {"Administration", showAdministrationMenu},
        {"Customize", showCustomizeMenu},
        {"Illegal Operation", showIllegalOperationMenu},
        {"Internet", showInternetMenu},
        {"System Information", showSystemInfoMenu},
        {"System Setting", showSystemSettingMenu},
        {"Terminstant Settings", showSettingsMenu},
        {"Utilities", showUtilitiesMenu}
    };

    mainMenu.executeMenu(mainMenuFunctions);

    endwin();
    return 0;
}

void showIllegalOperationMenu() {
    std::vector<std::string> options = {
        "Custom User's Menu",
        "Task Manager Killer",
        "Back"
    };
    Menu menu("=== Illegal Operation ===", options);
    
    // Function mapping for illegal operation menu
    std::map<std::string, std::function<void()>> illegalMenuFunctions = {
        {"Custom User's Menu", []() { showCustomUserMenus("Illegal Operation"); }},
        {"Task Manager Killer", showTaskKillerMenu}
    };
    
    menu.executeMenu(illegalMenuFunctions);
}

void showAdministrationMenu() {
    std::vector<std::string> options = {
        "Custom User's Menu",
        "Package Management via APT",
        "Sudo Settings",
        "Task Manager (htop)",
        "Back"
    };
    Menu menu("=== Administration ===", options);
    
    // Function mapping for administration menu
    std::map<std::string, std::function<void()>> adminMenuFunctions = {
        {"Custom User's Menu", []() { showCustomUserMenus("Administration"); }},
        {"Package Management via APT", showAptPackageManagementMenu},
        {"Sudo Settings", showSudoSettingsMenu},
        {"Task Manager (htop)", showHtopMenu}
    };
    
    menu.executeMenu(adminMenuFunctions);
}



void showSystemSettingMenu() {
    std::vector<std::string> options = {
        "Custom User's Menu",
        "Input Devices (xinput)",
        "Back"
    };
    Menu menu("=== System Setting ===", options);
    
    // Function mapping for system setting menu
    std::map<std::string, std::function<void()>> systemSettingFunctions = {
        {"Custom User's Menu", []() { showCustomUserMenus("System Setting"); }},
        {"Input Devices (xinput)", showInputDevicesMenu}
    };
    
    menu.executeMenu(systemSettingFunctions);
}

void showUtilitiesMenu() {
    std::vector<std::string> options = {
        "Custom User's Menu",
        "Manage Files and Folders",
        "Tar Program",
        "Back"
    };
    Menu menu("=== Utilities ===", options);
    
    // Function mapping for utilities menu
    std::map<std::string, std::function<void()>> utilitiesFunctions = {
        {"Custom User's Menu", []() { showCustomUserMenus("Utilities"); }},
        {"Manage Files and Folders", showLsMenu}
        // {"Tar Program", showTarProgramMenu} // Uncomment when implemented
    };
    
    menu.executeMenu(utilitiesFunctions);
}

void showSystemInfoMenu() {
    std::vector<std::string> options = {
        "Custom User's Menu",
        "Screenfetch",
        "Block Devices (lsblk)",
        "Information via uname",
        "Back"
    };
    Menu menu("=== System Information ===", options);
    
    // Function mapping for system info menu
    std::map<std::string, std::function<void()>> systemInfoFunctions = {
        {"Custom User's Menu", []() { showCustomUserMenus("System Information"); }},
        {"Block Devices (lsblk)", showLsblkMenu},
        {"Information via uname", showUnameInfoMenu},
        {"Screenfetch", showScreenfetchMenu}
    };
    
    menu.executeMenu(systemInfoFunctions);
}

void showInternetMenu() {
    std::vector<std::string> options = {
        "Custom User's Menu",
        "Network Monitor (iftop)",
        "Back"
    };
    Menu menu("=== Internet ===", options);
    
    // Function mapping for internet menu
    std::map<std::string, std::function<void()>> internetFunctions = {
        {"Custom User's Menu", []() { showCustomUserMenus("Internet"); }},
        {"Network Monitor (iftop)", showNetworkMonitorMenu}
    };
    
    menu.executeMenu(internetFunctions);
}

void showCustomizeMenu() {
    std::vector<std::string> options = {
        "Change Header Text",
        "Manage User's Menu",
        "Back"
    };
    Menu menu("=== Customize ===", options);
    
    // Function mapping for customize menu
    std::map<std::string, std::function<void()>> customizeFunctions = {
        {"Change Header Text", showHeaderMenu},
        {"Manage User's Menu", showMenuManagementMenu}
    };
    
    menu.executeMenu(customizeFunctions);
}

void showCustomUserMenus(const std::string& parentMenu) {
    std::vector<std::string> customMenuNames = getCustomMenusForParent(parentMenu);
    
    if (customMenuNames.empty()) {
        // No custom menus for this parent
        std::vector<std::string> options = {"Back"};
        Menu menu("=== Custom User's Menu ===", options);
        
        clear();
        refresh();
        mvprintw(1, 0, "=== Custom User's Menu ===");
        mvprintw(3, 0, "No custom menus found for %s.", parentMenu.c_str());
        mvprintw(4, 0, "Use Customize -> Manage User's Menu -> Add Menu to create custom menus.");
        mvprintw(6, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Add Back option
    customMenuNames.push_back("Back");
    
    Menu menu("=== Custom User's Menu ===", customMenuNames);
    
    // Create function mapping for custom menus
    std::map<std::string, std::function<void()>> customMenuFunctions;
    std::vector<std::string> originalNames = getCustomMenusForParent(parentMenu);
    
    for (const std::string& menuName : originalNames) {
        customMenuFunctions[menuName] = [menuName]() {
            executeCustomMenu(menuName);
        };
    }
    
    menu.executeMenu(customMenuFunctions);
}
