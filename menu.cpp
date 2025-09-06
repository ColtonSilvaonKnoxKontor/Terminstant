// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#include "menu.hpp"
#include <string>
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <algorithm>
#include <unistd.h>
#include <cstdlib>
#include <pwd.h>
#include <sys/types.h>

std::string Menu::defaultHelpText = "";
int Menu::defaultHelpColorPair = 0;
std::string Menu::defaultHeaderText = "";
int Menu::defaultHeaderColorPair = 0;

// Global current directory tracking
static std::string currentWorkingDir;

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
        // Draw current directory status above help bar
        std::string currentDir = getCurrentDirectory();
        if (!currentDir.empty() && yMax > 2) {
            if (has_colors()) attron(COLOR_PAIR(5)); // Green color
            std::string dirText = "Current Directory: " + currentDir;
            int available = xMax - 2;
            if (available < 0) available = 0;
            if ((int)dirText.size() > available) dirText = dirText.substr(0, (size_t)available);
            mvprintw(yMax - 2, 1, "%s", dirText.c_str());
            if (has_colors()) attroff(COLOR_PAIR(5));
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

void showScrollableOutput(const std::string& command, const std::string& title) {
    clear();
    refresh();
    
    // Collect all output lines first
    std::vector<std::string> lines;
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[2048];
        while (fgets(buffer, sizeof(buffer), pipe)) {
            std::string line = buffer;
            // Remove carriage returns and trailing newlines
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }
            lines.push_back(line);
        }
        pclose(pipe);
    }
    
    // Handle scrolling
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    int displayLines = maxLines - 3; // Reserve space for title and help text
    int startLine = 0;
    int totalLines = (int)lines.size();
    
    // If we have a title, reserve one more line
    if (!title.empty()) {
        displayLines -= 1;
    }
    
    int c;
    while (true) {
        clear();
        
        // Display title if provided
        int currentRow = 0;
        if (!title.empty()) {
            mvprintw(currentRow, 0, "%s", title.c_str());
            currentRow++;
        }
        
        // Display visible lines
        for (int i = 0; i < displayLines && (startLine + i) < totalLines; i++) {
            std::string line = lines[startLine + i];
            // Truncate line to fit screen width
            if ((int)line.size() > maxCols - 1) {
                line = line.substr(0, (size_t)(maxCols - 1));
            }
            mvprintw(currentRow + i, 0, "%s", line.c_str());
        }
        
        // Display help text and scroll info
        std::string helpText = "Up/Down: Scroll  PgUp/PgDn: Page  ESC: Exit";
        if (totalLines > displayLines) {
            helpText += " | Line " + std::to_string(startLine + 1) + "-" + 
                       std::to_string(std::min(startLine + displayLines, totalLines)) + 
                       " of " + std::to_string(totalLines);
        }
        mvprintw(maxLines - 1, 0, "%s", helpText.c_str());
        
        refresh();
        c = getch();
        
        switch (c) {
            case KEY_UP:
                if (startLine > 0) startLine--;
                break;
            case KEY_DOWN:
                if (startLine + displayLines < totalLines) startLine++;
                break;
            case KEY_PPAGE: // Page Up
                startLine -= displayLines;
                if (startLine < 0) startLine = 0;
                break;
            case KEY_NPAGE: // Page Down
                startLine += displayLines;
                if (startLine + displayLines > totalLines) {
                    startLine = std::max(0, totalLines - displayLines);
                }
                break;
            case KEY_HOME: // Home key - go to top
                startLine = 0;
                break;
            case KEY_END: // End key - go to bottom
                startLine = std::max(0, totalLines - displayLines);
                break;
            case 27: // ESC
                return;
            case KEY_RESIZE: // Handle terminal resize
                getmaxyx(stdscr, maxLines, maxCols);
                displayLines = maxLines - 3;
                if (!title.empty()) displayLines -= 1;
                if (startLine + displayLines > totalLines) {
                    startLine = std::max(0, totalLines - displayLines);
                }
                break;
        }
    }
}

// Helper function to get user's home directory
static std::string getHomeDirectory() {
    // First try the HOME environment variable
    const char* home = getenv("HOME");
    if (home != nullptr) {
        return std::string(home);
    }
    
    // Fallback to getpwuid if HOME is not set
    struct passwd* pw = getpwuid(getuid());
    if (pw != nullptr && pw->pw_dir != nullptr) {
        return std::string(pw->pw_dir);
    }
    
    // Last resort fallback
    return "/tmp";
}

// Current directory tracking functions
std::string getCurrentDirectory() {
    if (currentWorkingDir.empty()) {
        // Initialize with user's home directory instead of current working directory
        currentWorkingDir = getHomeDirectory();
        // Also change the actual process directory to home
        chdir(currentWorkingDir.c_str());
    }
    return currentWorkingDir;
}

void updateCurrentDirectory(const std::string& newDir) {
    currentWorkingDir = newDir;
    // Also change the actual process directory
    chdir(newDir.c_str());
}

void initializeHomeDirectory() {
    // Force initialization of the home directory
    currentWorkingDir = getHomeDirectory();
    chdir(currentWorkingDir.c_str());
}

void executeInteractiveCommand(const std::string& command, const std::string& title) {
    clear();
    refresh();
    
    // Display title if provided
    int currentRow = 0;
    if (!title.empty()) {
        mvprintw(currentRow, 0, "%s", title.c_str());
        currentRow += 2;
    }
    
    // Display current directory
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(currentRow, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    currentRow += 2;
    
    // Display the command being executed
    if (has_colors()) attron(COLOR_PAIR(6)); // Yellow color
    mvprintw(currentRow, 0, "Executing: %s", command.c_str());
    if (has_colors()) attroff(COLOR_PAIR(6));
    currentRow += 2;
    
    mvprintw(currentRow, 0, "Output:");
    currentRow++;
    
    refresh();
    
    // Execute command using system() which handles interactive input
    // First, we need to temporarily end ncurses mode for the command
    endwin();
    
    // Execute the command - this will handle interactive prompts naturally
    int result = system(command.c_str());
    
    // Reinitialize ncurses
    initializeCurses();
    
    clear();
    
    // Show completion status
    if (!title.empty()) {
        mvprintw(0, 0, "%s", title.c_str());
        currentRow = 2;
    } else {
        currentRow = 0;
    }
    
    // Display current directory again
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(currentRow, 0, "Current Directory: %s", getCurrentDirectory().c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    currentRow += 2;
    
    // Display the command that was executed
    if (has_colors()) attron(COLOR_PAIR(6)); // Yellow color
    mvprintw(currentRow, 0, "Executed: %s", command.c_str());
    if (has_colors()) attroff(COLOR_PAIR(6));
    currentRow += 2;
    
    // Show completion status with color
    if (result == 0) {
        if (has_colors()) attron(COLOR_PAIR(5)); // Green for success
        mvprintw(currentRow, 0, "Command completed successfully.");
        if (has_colors()) attroff(COLOR_PAIR(5));
    } else {
        if (has_colors()) attron(COLOR_PAIR(4)); // Red for error
        mvprintw(currentRow, 0, "Command completed with exit code: %d", result);
        if (has_colors()) attroff(COLOR_PAIR(4));
    }
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}
