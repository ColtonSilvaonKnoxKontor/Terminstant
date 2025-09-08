// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#pragma once
#include <string>
#include <vector>
#include <functional>
#include <map>

class Menu {
public:
    Menu(const std::string &title, const std::vector<std::string> &options);
    int run();
    void setHelp(const std::string &text, int colorPair);
    static void setGlobalHelp(const std::string &text, int colorPair);
    void setHeader(const std::string &text, int colorPair);
    static void setGlobalHeader(const std::string &text, int colorPair);
    static void handleResize();
    
    // Dynamic menu execution with function mapping
    void executeMenu(const std::map<std::string, std::function<void()>>& functionMap);
    
    // Get option by index (after sorting)
    std::string getOption(int index) const;
    
private:
    std::string title;
    std::vector<std::string> options;
    std::string helpText;
    int helpColorPair;
    std::string headerText;
    int headerColorPair;
    static std::string defaultHelpText;
    static int defaultHelpColorPair;
    static std::string defaultHeaderText;
    static int defaultHeaderColorPair;
    
    // Helper function to sort options alphabetically
    void sortOptions();
};

// Shared ncurses initializer to be used across all modules after returning from system() calls
void initializeCurses();
void setupResizeHandler();

// Scrollable output function for displaying command output
void showScrollableOutput(const std::string& command, const std::string& title = "");

// Interactive command execution function for commands that require user input
void executeInteractiveCommand(const std::string& command, const std::string& title = "");

// Current directory tracking
std::string getCurrentDirectory();
void updateCurrentDirectory(const std::string& newDir);
void initializeHomeDirectory();
