// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#ifndef SUDO_SETTINGS_HPP
#define SUDO_SETTINGS_HPP

#include <string>

// Function declarations for sudo settings menu
void showSudoSettingsMenu();
void changeToSu();
void changeToSudo();
void showCurrentSetting();
void saveSudoSetting();
void loadSudoSetting();
std::string getSudoCommand();

// Global variable to track current sudo method
extern std::string sudo_method;

#endif // SUDO_SETTINGS_HPP
