// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#include <ncurses.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "../menu.hpp"
#include <cstdlib>  // system()

void showHtopMenu() {
    std::vector<std::string> htopOptions = {
        "Run htop (default)",
        "Run htop with no mouse",
        "Run htop in tree view",
        "Run htop in monochrome color scheme",
        "Back"
    };

    Menu htopMenu("===Htop Menu===", htopOptions);

    // Function mapping for htop menu
    std::map<std::string, std::function<void()>> htopFunctions = {
        {"Run htop (default)", []() {
            endwin();
            system("htop");
            initializeCurses();
            setupResizeHandler();
        }},
        {"Run htop with no mouse", []() {
            endwin();
            system("htop --no-mouse");
            initializeCurses();
            setupResizeHandler();
        }},
        {"Run htop in tree view", []() {
            endwin();
            system("htop --tree");
            initializeCurses();
            setupResizeHandler();
        }},
        {"Run htop in monochrome color scheme", []() {
            endwin();
            system("htop --no-color");
            initializeCurses();
            setupResizeHandler();
        }}
    };

    htopMenu.executeMenu(htopFunctions);
}
