// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#ifndef TERM_MENU_H
#define TERM_MENU_H

#include <string>
#include <vector>

// Main function to show the menu management interface
void showMenuManagementMenu();

// Function to initialize custom menus at startup
void initializeCustomMenus();

// Function to get custom menus for a specific parent menu
std::vector<std::string> getCustomMenusForParent(const std::string& parentMenu);

// Function to execute a custom menu
void executeCustomMenu(const std::string& menuName);

#endif
