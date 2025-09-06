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

int main() {
    initializeCurses();
    setupResizeHandler();
    loadSudoSetting();

    std::vector<std::string> mainOptions = {
        "Illegal Operation",
        "Administration",
        "System Setting",
        "System Information",
        "Utilities",
        "Internet",
        "Terminstant Settings",
        "Exit"
    };

    Menu::setGlobalHelp("Up/Down: Navigate  Enter: Select  ESC: Back/Exit", 1);
    Menu::setGlobalHeader("Terminstant v1.0 (2025 SILVASYSTEMS by Colton Silva)", 2);
    Menu mainMenu("=== Main Menu ===", mainOptions);

    // Function mapping for main menu
    std::map<std::string, std::function<void()>> mainMenuFunctions = {
        {"Administration", showAdministrationMenu},
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
        "Task Manager Killer",
        "Back"
    };
    Menu menu("=== Illegal Operation ===", options);
    
    // Function mapping for illegal operation menu
    std::map<std::string, std::function<void()>> illegalMenuFunctions = {
        {"Task Manager Killer", showTaskKillerMenu}
    };
    
    menu.executeMenu(illegalMenuFunctions);
}

void showAdministrationMenu() {
    std::vector<std::string> options = {
        "Package Management via APT",
        "Sudo Settings",
        "Task Manager (htop)",
        "Back"
    };
    Menu menu("=== Administration ===", options);
    
    // Function mapping for administration menu
    std::map<std::string, std::function<void()>> adminMenuFunctions = {
        {"Package Management via APT", showAptPackageManagementMenu},
        {"Sudo Settings", showSudoSettingsMenu},
        {"Task Manager (htop)", showHtopMenu}
    };
    
    menu.executeMenu(adminMenuFunctions);
}



void showSystemSettingMenu() {
    std::vector<std::string> options = {
        "Input Devices (xinput)",
        "Back"
    };
    Menu menu("=== System Setting ===", options);
    
    // Function mapping for system setting menu
    std::map<std::string, std::function<void()>> systemSettingFunctions = {
        {"Input Devices (xinput)", showInputDevicesMenu}
    };
    
    menu.executeMenu(systemSettingFunctions);
}

void showUtilitiesMenu() {
    std::vector<std::string> options = {
        "Manage Files and Folders",
        "Tar Program",
        "Back"
    };
    Menu menu("=== Utilities ===", options);
    
    // Function mapping for utilities menu
    std::map<std::string, std::function<void()>> utilitiesFunctions = {
        {"Manage Files and Folders", showLsMenu}
        // {"Tar Program", showTarProgramMenu} // Uncomment when implemented
    };
    
    menu.executeMenu(utilitiesFunctions);
}

void showSystemInfoMenu() {
    std::vector<std::string> options = {
        "Screenfetch",
        "Block Devices (lsblk)",
        "Information via uname",
        "Back"
    };
    Menu menu("=== System Information ===", options);
    
    // Function mapping for system info menu
    std::map<std::string, std::function<void()>> systemInfoFunctions = {
        {"Block Devices (lsblk)", showLsblkMenu},
        {"Information via uname", showUnameInfoMenu},
        {"Screenfetch", showScreenfetchMenu}
    };
    
    menu.executeMenu(systemInfoFunctions);
}

void showInternetMenu() {
    std::vector<std::string> options = {
        "Network Monitor (iftop)",
        "Back"
    };
    Menu menu("=== Internet ===", options);
    
    // Function mapping for internet menu
    std::map<std::string, std::function<void()>> internetFunctions = {
        {"Network Monitor (iftop)", showNetworkMonitorMenu}
    };
    
    menu.executeMenu(internetFunctions);
}
