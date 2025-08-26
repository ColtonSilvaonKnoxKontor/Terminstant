// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#include "menu.hpp"
#include <string>
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <algorithm>

std::string Menu::defaultHelpText = "";
int Menu::defaultHelpColorPair = 0;
std::string Menu::defaultHeaderText = "";
int Menu::defaultHeaderColorPair = 0;

Menu::Menu(const std::string &t, const std::vector<std::string> &opts)
    : title(t), options(opts), helpText(""), helpColorPair(0), headerText(""), headerColorPair(0) {
    sortOptions();
}

void Menu::setHelp(const std::string &text, int colorPair) {
    helpText = text;
    helpColorPair = colorPair;
}

void Menu::setGlobalHelp(const std::string &text, int colorPair) {
    defaultHelpText = text;
    defaultHelpColorPair = colorPair;
}

void Menu::setHeader(const std::string &text, int colorPair) {
    headerText = text;
    headerColorPair = colorPair;
}

void Menu::setGlobalHeader(const std::string &text, int colorPair) {
    defaultHeaderText = text;
    defaultHeaderColorPair = colorPair;
}

int Menu::run() {
    clear();
    int highlight = 0;
    int choice = 0;
    int c;

    while (true) {
        clear();
        // Draw header bar at top (centered) using instance or global default
        const std::string &topText = headerText.empty() ? defaultHeaderText : headerText;
        const int topColor = (headerText.empty() ? defaultHeaderColorPair : headerColorPair);
        int yMax, xMax;
        getmaxyx(stdscr, yMax, xMax);
        if (!topText.empty() && yMax > 0 && xMax > 0) {
            if (has_colors() && topColor > 0) attron(COLOR_PAIR(topColor));
            mvhline(0, 0, ' ', xMax);
            int available = xMax - 2;
            if (available < 0) available = 0;
            std::string text = topText;
            if ((int)text.size() > available) text = text.substr(0, (size_t)available);
            int startX = (xMax - (int)text.size()) / 2;
            if (startX < 0) startX = 0;
            mvprintw(0, startX, "%s", text.c_str());
            if (has_colors() && topColor > 0) attroff(COLOR_PAIR(topColor));
        }
        // Title row below header if header exists
        int titleRow = (!topText.empty() ? 1 : 0);
        mvprintw(titleRow, 0, "%s", title.c_str());
        for (size_t i = 0; i < options.size(); ++i) {
            if ((int)i == highlight)
                attron(A_REVERSE);
            mvprintw((int)i + 1 + titleRow, 0, "%s", options[i].c_str());
            if ((int)i == highlight)
                attroff(A_REVERSE);
        }
        // Draw help bar at the bottom using instance or global default
        const std::string &barText = helpText.empty() ? defaultHelpText : helpText;
        const int barColor = (helpText.empty() ? defaultHelpColorPair : helpColorPair);
        if (!barText.empty() && yMax > 0 && xMax > 0) {
            if (has_colors() && barColor > 0) attron(COLOR_PAIR(barColor));
            mvhline(yMax - 1, 0, ' ', xMax);
            int available = xMax - 2;
            if (available < 0) available = 0;
            std::string text = barText;
            if ((int)text.size() > available) text = text.substr(0, (size_t)available);
            mvprintw(yMax - 1, 1, "%s", text.c_str());
            if (has_colors() && barColor > 0) attroff(COLOR_PAIR(barColor));
        }
        c = getch();
        switch (c) {
            case KEY_UP:
                highlight = (highlight == 0) ? (int)options.size() - 1 : highlight - 1;
                break;
            case KEY_DOWN:
                highlight = (highlight == (int)options.size() - 1) ? 0 : highlight + 1;
                break;
            case 10: // Enter
                return highlight;
            case 27: // ESC to quit
                return -1;
            case KEY_RESIZE: // Handle terminal resize
                handleResize();
                break;
        }
    }

}

void initializeCurses() {
    // Enable UTF-8 handling for wide characters (e.g., box-drawing ├─, └─)
    setlocale(LC_ALL, "");
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_BLACK, COLOR_CYAN);      // Help bar
        init_pair(2, COLOR_BLACK, COLOR_YELLOW);    // Header bar
        init_pair(3, COLOR_WHITE, COLOR_BLACK);     // Default text
        init_pair(4, COLOR_RED, COLOR_BLACK);       // Red text
        init_pair(5, COLOR_GREEN, COLOR_BLACK);     // Green text
        init_pair(6, COLOR_YELLOW, COLOR_BLACK);    // Yellow text
        init_pair(7, COLOR_BLUE, COLOR_BLACK);      // Blue text
        init_pair(8, COLOR_MAGENTA, COLOR_BLACK);   // Magenta text
        init_pair(9, COLOR_CYAN, COLOR_BLACK);      // Cyan text
        init_pair(10, COLOR_WHITE, COLOR_BLACK);    // Bright white
    }
}

void setupResizeHandler() {
    // Enable window resize detection
    resizeterm(0, 0); // This triggers a resize event to refresh the screen
}

void Menu::handleResize() {
    // Clear and refresh the screen when terminal is resized
    clear();
    refresh();
}

void Menu::sortOptions() {
    // Sort options alphabetically, case-insensitive
    // Keep "Back" and "Exit" at the end if they exist
    std::vector<std::string> sortedOptions;
    std::vector<std::string> endOptions;
    
    for (const auto& option : options) {
        if (option == "Back" || option == "Exit") {
            endOptions.push_back(option);
        } else {
            sortedOptions.push_back(option);
        }
    }
    
    // Sort the main options alphabetically (case-insensitive)
    std::sort(sortedOptions.begin(), sortedOptions.end(), 
        [](const std::string& a, const std::string& b) {
            std::string a_lower = a;
            std::string b_lower = b;
            std::transform(a_lower.begin(), a_lower.end(), a_lower.begin(), ::tolower);
            std::transform(b_lower.begin(), b_lower.end(), b_lower.begin(), ::tolower);
            return a_lower < b_lower;
        });
    
    // Combine sorted options with end options
    options.clear();
    options.insert(options.end(), sortedOptions.begin(), sortedOptions.end());
    options.insert(options.end(), endOptions.begin(), endOptions.end());
}

void Menu::executeMenu(const std::map<std::string, std::function<void()>>& functionMap) {
    int choice;
    while ((choice = run()) != -1) {
        if (choice >= 0 && choice < (int)options.size()) {
            std::string selectedOption = options[choice];
            
            // Check if the selected option has a corresponding function
            auto it = functionMap.find(selectedOption);
            if (it != functionMap.end()) {
                it->second(); // Execute the function
            } else if (selectedOption == "Back" || selectedOption == "Exit") {
                return; // Exit the menu
            }
        }
    }
}
