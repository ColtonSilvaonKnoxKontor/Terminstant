// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <string>
#include <vector>

// Structure to hold program information
struct ProgramInfo {
    std::string name;
    std::string package_name;
    std::string description;
    bool is_installed;
    std::string install_command;
};

// Function declarations for settings menu
void showSettingsMenu();
void showProgramsMenu();
void showDependencyChecker();
void showInstalledPrograms();
void installMissingPrograms(const std::vector<ProgramInfo>& programs);

#endif // SETTINGS_HPP
