// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// term_menu module for managing custom user menus

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
#include <dirent.h>
#include <cstring>
#include "../menu.hpp"

using namespace std;

// Configuration constants
static const std::string CONFIG_DIR_NAME = ".terminstant";
static const std::string CUSTOM_MENUS_DIR = "custom_menus";

// Struct to represent a custom menu command
struct CustomCommand {
    std::string name;
    std::string baseCommand;
    std::vector<std::string> options;
    std::vector<std::string> arguments; // Argument prompts
    std::vector<std::string> subcommands;
    std::string destination;
    std::vector<std::string> pipelines;
    std::vector<std::string> operands;
    std::string executionType; // "scrollable", "interactive", "direct"
};

// Struct to represent a custom menu
struct CustomMenu {
    std::string name;
    std::string parentMenu;
    std::vector<CustomCommand> commands;
};

// Global storage for custom menus
static std::vector<CustomMenu> customMenus;

// Function to get terminstant config directory path
static std::string getConfigDirPath() {
    char* home = getenv("HOME");
    if (home == nullptr) {
        return "/tmp/" + CONFIG_DIR_NAME; // Fallback
    }
    return std::string(home) + "/" + CONFIG_DIR_NAME;
}

// Function to get custom menus directory path
static std::string getCustomMenusDirPath() {
    return getConfigDirPath() + "/" + CUSTOM_MENUS_DIR;
}

// Function to ensure directory exists
static bool ensureDirectoryExists(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true; // Directory already exists
        } else {
            return false; // Path exists but is not a directory
        }
    }
    
    // Create directory with mode 0755
    if (mkdir(path.c_str(), 0755) == 0) {
        return true;
    }
    
    return false; // Failed to create directory
}

// Function to get user input with cancellation support
static std::string getUserInput(const std::string& prompt, const std::string& example = "", int maxLength = 80) {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(3, 0, "%s", prompt.c_str());
    if (!example.empty()) {
        mvprintw(4, 0, "Example: %s", example.c_str());
    }
    mvprintw(5, 0, "(Press ESC to cancel, Enter to confirm)");
    mvprintw(7, 0, "> ");
    
    // Enable echo and cursor for input
    echo();
    curs_set(1);
    
    char input[512];
    int ch;
    int pos = 0;
    
    while ((ch = getch()) != 27 && ch != 10 && pos < maxLength) { // ESC or Enter
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


// Function to save custom menu to file
static bool saveCustomMenu(const CustomMenu& menu) {
    if (!ensureDirectoryExists(getConfigDirPath())) {
        return false;
    }
    if (!ensureDirectoryExists(getCustomMenusDirPath())) {
        return false;
    }
    
    std::string menuDir = getCustomMenusDirPath() + "/" + menu.name;
    if (!ensureDirectoryExists(menuDir)) {
        return false;
    }
    
    std::string configPath = menuDir + "/menu.conf";
    std::ofstream configFile(configPath);
    
    if (!configFile.is_open()) {
        return false;
    }
    
    configFile << "# Custom Menu Configuration for: " << menu.name << "\n";
    configFile << "# Generated automatically - edit with caution\n";
    configFile << "menu_name=" << menu.name << "\n";
    configFile << "parent_menu=" << menu.parentMenu << "\n";
    configFile << "\n";
    
    for (size_t i = 0; i < menu.commands.size(); i++) {
        const CustomCommand& cmd = menu.commands[i];
        configFile << "[command_" << i << "]\n";
        configFile << "name=" << cmd.name << "\n";
        configFile << "base_command=" << cmd.baseCommand << "\n";
        configFile << "execution_type=" << cmd.executionType << "\n";
        
        if (!cmd.options.empty()) {
            configFile << "options=";
            for (size_t j = 0; j < cmd.options.size(); j++) {
                if (j > 0) configFile << ",";
                configFile << cmd.options[j];
            }
            configFile << "\n";
        }
        
        if (!cmd.arguments.empty()) {
            configFile << "arguments=";
            for (size_t j = 0; j < cmd.arguments.size(); j++) {
                if (j > 0) configFile << ",";
                configFile << cmd.arguments[j];
            }
            configFile << "\n";
        }
        
        if (!cmd.subcommands.empty()) {
            configFile << "subcommands=";
            for (size_t j = 0; j < cmd.subcommands.size(); j++) {
                if (j > 0) configFile << ",";
                configFile << cmd.subcommands[j];
            }
            configFile << "\n";
        }
        
        if (!cmd.destination.empty()) {
            configFile << "destination=" << cmd.destination << "\n";
        }
        
        if (!cmd.pipelines.empty()) {
            configFile << "pipelines=";
            for (size_t j = 0; j < cmd.pipelines.size(); j++) {
                if (j > 0) configFile << ",";
                configFile << cmd.pipelines[j];
            }
            configFile << "\n";
        }
        
        if (!cmd.operands.empty()) {
            configFile << "operands=";
            for (size_t j = 0; j < cmd.operands.size(); j++) {
                if (j > 0) configFile << ",";
                configFile << cmd.operands[j];
            }
            configFile << "\n";
        }
        
        configFile << "\n";
    }
    
    configFile.close();
    return true;
}

// Function to load custom menu from file
static CustomMenu loadCustomMenu(const std::string& menuPath) {
    CustomMenu menu;
    std::ifstream configFile(menuPath + "/menu.conf");
    
    if (!configFile.is_open()) {
        return menu; // Return empty menu
    }
    
    std::string line;
    CustomCommand currentCommand;
    bool inCommand = false;
    
    while (std::getline(configFile, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        if (line.find("menu_name=") == 0) {
            menu.name = line.substr(10);
        } else if (line.find("parent_menu=") == 0) {
            menu.parentMenu = line.substr(12);
        } else if (line.find("[command_") == 0) {
            if (inCommand) {
                menu.commands.push_back(currentCommand);
            }
            currentCommand = CustomCommand();
            inCommand = true;
        } else if (inCommand) {
            if (line.find("name=") == 0) {
                currentCommand.name = line.substr(5);
            } else if (line.find("base_command=") == 0) {
                currentCommand.baseCommand = line.substr(13);
            } else if (line.find("execution_type=") == 0) {
                currentCommand.executionType = line.substr(15);
            } else if (line.find("options=") == 0) {
                std::string optionsStr = line.substr(8);
                std::stringstream ss(optionsStr);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    currentCommand.options.push_back(item);
                }
            } else if (line.find("arguments=") == 0) {
                std::string argsStr = line.substr(10);
                std::stringstream ss(argsStr);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    currentCommand.arguments.push_back(item);
                }
            } else if (line.find("subcommands=") == 0) {
                std::string subcmdsStr = line.substr(12);
                std::stringstream ss(subcmdsStr);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    currentCommand.subcommands.push_back(item);
                }
            } else if (line.find("destination=") == 0) {
                currentCommand.destination = line.substr(12);
            } else if (line.find("pipelines=") == 0) {
                std::string pipelinesStr = line.substr(10);
                std::stringstream ss(pipelinesStr);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    currentCommand.pipelines.push_back(item);
                }
            } else if (line.find("operands=") == 0) {
                std::string operandsStr = line.substr(9);
                std::stringstream ss(operandsStr);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    currentCommand.operands.push_back(item);
                }
            }
        }
    }
    
    if (inCommand) {
        menu.commands.push_back(currentCommand);
    }
    
    configFile.close();
    return menu;
}

// Function to load all custom menus
static void loadAllCustomMenus() {
    customMenus.clear();
    
    std::string customMenusDir = getCustomMenusDirPath();
    DIR* dir = opendir(customMenusDir.c_str());
    
    if (dir == nullptr) {
        return; // Directory doesn't exist or can't be opened
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            std::string menuPath = customMenusDir + "/" + entry->d_name;
            CustomMenu menu = loadCustomMenu(menuPath);
            if (!menu.name.empty()) {
                customMenus.push_back(menu);
            }
        }
    }
    
    closedir(dir);
}

// Function to build command string (for preview only)
static std::string buildCommandPreview(const CustomCommand& cmd) {
    std::string fullCommand = cmd.baseCommand;
    
    // Add options first
    for (const std::string& option : cmd.options) {
        fullCommand += " " + option;
    }
    
    // Add subcommands
    for (const std::string& subcmd : cmd.subcommands) {
        fullCommand += " " + subcmd;
    }
    
    // Add argument placeholders AFTER options and subcommands
    for (const std::string& arg : cmd.arguments) {
        if (arg.find("__PROMPT__") == 0) {
            std::string promptText = arg.substr(10); // Remove "__PROMPT__" prefix
            fullCommand += " [" + promptText + "]";
        }
    }
    
    // Add operands
    for (const std::string& operand : cmd.operands) {
        fullCommand += " " + operand;
    }
    
    // Add destination
    if (!cmd.destination.empty()) {
        fullCommand += " " + cmd.destination;
    }
    
    // Add pipelines
    for (const std::string& pipeline : cmd.pipelines) {
        fullCommand += " | " + pipeline;
    }
    
    return fullCommand;
}

// Function to build command string with actual arguments
static std::string buildCommand(const CustomCommand& cmd, const std::string& userArguments = "") {
    std::string fullCommand = cmd.baseCommand;
    
    // Add options first
    for (const std::string& option : cmd.options) {
        fullCommand += " " + option;
    }
    
    // Add subcommands
    for (const std::string& subcmd : cmd.subcommands) {
        fullCommand += " " + subcmd;
    }
    
    // Add arguments AFTER options and subcommands
    if (!userArguments.empty()) {
        fullCommand += " " + userArguments;
    }
    
    // Add operands
    for (const std::string& operand : cmd.operands) {
        fullCommand += " " + operand;
    }
    
    // Add destination
    if (!cmd.destination.empty()) {
        fullCommand += " " + cmd.destination;
    }
    
    // Add pipelines
    for (const std::string& pipeline : cmd.pipelines) {
        fullCommand += " | " + pipeline;
    }
    
    return fullCommand;
}

// Function to execute custom command
static void executeCustomCommand(const CustomCommand& cmd) {
    std::string userArguments = "";
    
    // Collect arguments from multiple prompts
    for (const std::string& arg : cmd.arguments) {
        if (arg.find("__PROMPT__") == 0) {
            std::string promptText = arg.substr(10); // Remove "__PROMPT__" prefix
            std::string input = getUserInput(promptText + ":", "Enter value");
            if (input.empty()) {
                return; // User cancelled
            }
            if (!userArguments.empty()) {
                userArguments += " ";
            }
            userArguments += input;
        }
    }
    
    std::string fullCommand = buildCommand(cmd, userArguments);
    
    // Show what command will be executed
    clear();
    refresh();
    mvprintw(1, 0, "=== Executing: %s ===", cmd.name.c_str());
    mvprintw(3, 0, "Command: %s", fullCommand.c_str());
    mvprintw(5, 0, "Press any key to execute, ESC to cancel...");
    refresh();
    
    int ch = getch();
    if (ch == 27) { // ESC to cancel
        return;
    }
    
    // Always run commands with scrollable output so user can see results
    // Add some debugging to see if command is actually running
    clear();
    mvprintw(1, 0, "Executing command...");
    mvprintw(2, 0, "Command: %s", fullCommand.c_str());
    mvprintw(4, 0, "Please wait...");
    refresh();
    
    showScrollableOutput(fullCommand, "=== " + cmd.name + " - Output ===");
}

// Function to show current command preview
static void showCommandPreview(const CustomCommand& cmd) {
    clear();
    refresh();
    
    mvprintw(1, 0, "=== Command Preview ===");
    mvprintw(3, 0, "Name: %s", cmd.name.c_str());
    mvprintw(4, 0, "Base Command: %s", cmd.baseCommand.c_str());
    mvprintw(5, 0, "Execution Type: %s", cmd.executionType.c_str());
    
    std::string fullCommand = buildCommandPreview(cmd);
    mvprintw(7, 0, "Full Command Preview:");
    
    // Word wrap the command if it's too long
    int maxCols;
    getmaxyx(stdscr, maxCols, maxCols);
    int startRow = 8;
    int currentRow = startRow;
    int currentCol = 0;
    
    for (char c : fullCommand) {
        if (currentCol >= maxCols - 2 || c == ' ') {
            if (c == ' ' && currentCol >= maxCols - 10) {
                currentRow++;
                currentCol = 0;
                continue;
            }
        }
        mvaddch(currentRow, currentCol, c);
        currentCol++;
        if (currentCol >= maxCols - 2) {
            currentRow++;
            currentCol = 0;
        }
    }
    
    mvprintw(currentRow + 2, 0, "Components:");
    int compRow = currentRow + 3;
    
    if (!cmd.options.empty()) {
        mvprintw(compRow++, 2, "Options: %zu items", cmd.options.size());
        for (size_t i = 0; i < cmd.options.size() && i < 3; i++) {
            mvprintw(compRow++, 4, "- %s", cmd.options[i].c_str());
        }
        if (cmd.options.size() > 3) {
            mvprintw(compRow++, 4, "... and %zu more", cmd.options.size() - 3);
        }
    }
    
    if (!cmd.arguments.empty()) {
        mvprintw(compRow++, 2, "Arguments: %zu items", cmd.arguments.size());
        for (size_t i = 0; i < cmd.arguments.size() && i < 3; i++) {
            mvprintw(compRow++, 4, "- %s", cmd.arguments[i].c_str());
        }
    }
    
    if (!cmd.subcommands.empty()) {
        mvprintw(compRow++, 2, "Subcommands: %zu items", cmd.subcommands.size());
        for (const std::string& sub : cmd.subcommands) {
            mvprintw(compRow++, 4, "- %s", sub.c_str());
        }
    }
    
    if (!cmd.destination.empty()) {
        mvprintw(compRow++, 2, "Destination: %s", cmd.destination.c_str());
    }
    
    if (!cmd.pipelines.empty()) {
        mvprintw(compRow++, 2, "Pipelines: %zu items", cmd.pipelines.size());
        for (const std::string& pipe : cmd.pipelines) {
            mvprintw(compRow++, 4, "- | %s", pipe.c_str());
        }
    }
    
    if (!cmd.operands.empty()) {
        mvprintw(compRow++, 2, "Operands: %zu items", cmd.operands.size());
        for (const std::string& op : cmd.operands) {
            mvprintw(compRow++, 4, "- %s", op.c_str());
        }
    }
    
    mvprintw(compRow + 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

// Function to add multiple items with prompting
static void addMultipleItems(std::vector<std::string>& container, const std::string& itemType, const std::string& examples) {
    while (true) {
        clear();
        refresh();
        
        mvprintw(1, 0, "=== Add %s ===", itemType.c_str());
        mvprintw(3, 0, "Current %s (%zu items):", itemType.c_str(), container.size());
        
        for (size_t i = 0; i < container.size(); i++) {
            mvprintw(4 + i, 2, "%zu. %s", i + 1, container[i].c_str());
        }
        
        mvprintw(6 + container.size(), 0, "1. Add new %s", itemType.c_str());
        mvprintw(7 + container.size(), 0, "2. Remove %s (by number)", itemType.c_str());
        mvprintw(8 + container.size(), 0, "3. Clear all %s", itemType.c_str());
        mvprintw(9 + container.size(), 0, "4. Done");
        mvprintw(11 + container.size(), 0, "Choice: ");
        
        int ch = getch();
        
        if (ch == '1') {
            std::string input = getUserInput("Enter " + itemType + ":", examples);
            if (!input.empty()) {
                container.push_back(input);
            }
        } else if (ch == '2' && !container.empty()) {
            std::string indexStr = getUserInput("Enter number to remove (1-" + std::to_string(container.size()) + "):", "1");
            if (!indexStr.empty()) {
                try {
                    int index = std::stoi(indexStr) - 1;
                    if (index >= 0 && index < (int)container.size()) {
                        container.erase(container.begin() + index);
                    }
                } catch (...) {
                    // Invalid input, ignore
                }
            }
        } else if (ch == '3') {
            container.clear();
        } else if (ch == '4') {
            break;
        }
    }
}

// Enhanced function to add format components to command
static void addCommandComponents(CustomCommand& cmd) {
    // Use ordered vector to maintain proper order without sorting issues
    std::vector<std::pair<std::string, int>> componentOrder = {
        {"Add Argument Prompt (add before/after options/subcommands)", 0},
        {"Destination (output: > file.txt, /path/to/)", 1},
        {"Operands (input files: *.txt, file1 file2)", 2},
        {"Options (flags: -v, --verbose, -rf)", 3},
        {"Pipelines (| grep, | sort, | head -10)", 4},
        {"Subcommands (git commit, docker run)", 5},
        {"Preview Command", 6},
        {"Done - Finish Adding Components", 7}
    };
    
    // Create mapping for quick lookup
    std::map<std::string, int> componentMapping;
    std::vector<std::string> componentTypes;
    for (const auto& pair : componentOrder) {
        componentTypes.push_back(pair.first);
        componentMapping[pair.first] = pair.second;
    }
    
    while (true) {
        // Show current component status
        clear();
        refresh();
        
        mvprintw(1, 0, "=== Command Components for: %s ===", cmd.name.c_str());
        mvprintw(3, 0, "Base Command: %s", cmd.baseCommand.c_str());
        
        mvprintw(5, 0, "Current Components:");
        int statusRow = 6;
        mvprintw(statusRow++, 2, "Options: %zu items", cmd.options.size());
        mvprintw(statusRow++, 2, "Argument Prompts: %zu", cmd.arguments.size());
        mvprintw(statusRow++, 2, "Subcommands: %zu items", cmd.subcommands.size());
        mvprintw(statusRow++, 2, "Destination: %s", cmd.destination.empty() ? "(none)" : cmd.destination.c_str());
        mvprintw(statusRow++, 2, "Pipelines: %zu items", cmd.pipelines.size());
        mvprintw(statusRow++, 2, "Operands: %zu items", cmd.operands.size());
        
        mvprintw(statusRow + 1, 0, "Select component type to configure:");
        
        Menu componentMenu("=== Add Command Components ===", componentTypes);
        int choice = componentMenu.run();
        
        if (choice == -1) break; // Cancelled
        
        // Get the actual selected option
        std::string selectedOption = componentMenu.getOption(choice);
        auto it = componentMapping.find(selectedOption);
        
        if (it == componentMapping.end()) continue;
        
        switch (it->second) {
            case 0: // Add Argument Prompt
                {
                    std::string prompt = getUserInput("Enter descriptive prompt for this argument:", "Enter filename, Enter port number, Enter target IP");
                    if (!prompt.empty()) {
                        cmd.arguments.push_back("__PROMPT__" + prompt);
                        clear();
                        mvprintw(1, 0, "Argument prompt added: %s", prompt.c_str());
                        mvprintw(2, 0, "When you run this command, you will be prompted: %s", prompt.c_str());
                        mvprintw(4, 0, "You can add multiple argument prompts.");
                        mvprintw(5, 0, "They will appear in the order you add them.");
                        mvprintw(7, 0, "Press any key to continue...");
                        refresh();
                        getch();
                    }
                }
                break;
                
            case 1: // Destination
                {
                    std::string input = getUserInput("Enter destination:", "> output.txt, >> log.txt, /path/to/output/");
                    if (!input.empty()) {
                        cmd.destination = input;
                    }
                }
                break;
                
            case 2: // Operands
                addMultipleItems(cmd.operands, "operands", "*.txt, file1.txt file2.txt, /path/to/data/, $(find . -name '*.log')");
                break;
                
            case 3: // Options
                addMultipleItems(cmd.options, "options", "-v, --verbose, -l, --long, -rf, --help");
                break;
                
            case 4: // Pipelines
                addMultipleItems(cmd.pipelines, "pipelines", "grep 'pattern', sort -n, head -10, tail -f, awk '{print $1}'");
                break;
                
            case 5: // Subcommands
                addMultipleItems(cmd.subcommands, "subcommands", "commit, push, pull, status, run, build");
                break;
                
            case 6: // Preview Command
                showCommandPreview(cmd);
                break;
                
            case 7: // Done
                return;
        }
    }
}


// Function to validate command name
static bool isValidCommandName(const std::string& name) {
    if (name.empty()) return false;
    if (name.length() > 50) return false;
    
    // Check for reserved names
    std::vector<std::string> reserved = {"Back", "Exit", "Menu", "Settings"};
    for (const std::string& res : reserved) {
        if (name == res) return false;
    }
    
    return true;
}

// Enhanced function to create new custom command
static CustomCommand createCustomCommand() {
    CustomCommand cmd;
    
    // Command name with validation
    while (true) {
        cmd.name = getUserInput("Enter command name (how it appears in menu):", "System Monitor, Git Status, Quick Backup");
        if (cmd.name.empty()) return cmd; // Cancelled
        
        if (isValidCommandName(cmd.name)) {
            break;
        }
        
        clear();
        mvprintw(1, 0, "Invalid command name!");
        mvprintw(2, 0, "- Must not be empty");
        mvprintw(3, 0, "- Must be 50 characters or less");
        mvprintw(4, 0, "- Cannot be: Back, Exit, Menu, Settings");
        mvprintw(5, 0, "Press any key to try again...");
        refresh();
        getch();
    }
    
    // Base command with validation and examples
    while (true) {
        clear();
        refresh();
        mvprintw(1, 0, "=== Enter Base Command ===");
        mvprintw(3, 0, "Enter the main command (without arguments/options):");
        mvprintw(5, 0, "Examples:");
        mvprintw(6, 2, "System commands: ls, ps, top, df, du, find");
        mvprintw(7, 2, "Development: git, make, gcc, python3, node");
        mvprintw(8, 2, "Network: ping, curl, wget, ssh, scp");
        mvprintw(9, 2, "Text processing: grep, awk, sed, sort, uniq");
        mvprintw(10, 2, "System admin: systemctl, service, mount, umount");
        
        cmd.baseCommand = getUserInput("Base command:", "ls, git, docker, python3, systemctl");
        if (cmd.baseCommand.empty()) return cmd; // Cancelled
        
        // Basic validation - ensure it's not just spaces
        if (cmd.baseCommand.find_first_not_of(" \t") != std::string::npos) {
            break;
        }
        
        clear();
        mvprintw(1, 0, "Error: Base command cannot be empty or just spaces!");
        mvprintw(2, 0, "Press any key to try again...");
        refresh();
        getch();
    }
    
    // Set default execution type to scrollable
    cmd.executionType = "scrollable";
    
    // Show summary before adding components
    clear();
    refresh();
    mvprintw(1, 0, "=== Command Summary ===");
    mvprintw(3, 0, "Name: %s", cmd.name.c_str());
    mvprintw(4, 0, "Base Command: %s", cmd.baseCommand.c_str());
    mvprintw(5, 0, "Execution Type: %s", cmd.executionType.c_str());
    mvprintw(7, 0, "Next, you can add optional components (flags, arguments, etc.)");
    mvprintw(8, 0, "This allows you to build complex commands like:");
    mvprintw(9, 2, "ls -la --color=auto /home/user");
    mvprintw(10, 2, "git log --oneline --graph | head -20");
    mvprintw(11, 2, "find /var/log -name '*.log' -type f");
    mvprintw(13, 0, "Press any key to continue to component configuration...");
    refresh();
    getch();
    
    // Add command components
    addCommandComponents(cmd);
    
    // Final confirmation and preview
    clear();
    refresh();
    mvprintw(1, 0, "=== Final Command Preview ===");
    std::string fullCommand = buildCommand(cmd);
    mvprintw(3, 0, "Final command that will be executed:");
    mvprintw(5, 0, "%s", fullCommand.c_str());
    mvprintw(7, 0, "Does this look correct?");
    mvprintw(8, 0, "y/Y - Yes, create this command");
    mvprintw(9, 0, "n/N - No, start over");
    mvprintw(10, 0, "Any other key - Cancel");
    mvprintw(12, 0, "Choice: ");
    refresh();
    
    int confirmChoice = getch();
    if (confirmChoice == 'y' || confirmChoice == 'Y') {
        return cmd; // Confirmed
    } else if (confirmChoice == 'n' || confirmChoice == 'N') {
        return createCustomCommand(); // Start over (recursive)
    } else {
        cmd.name.clear(); // Mark as cancelled
        return cmd;
    }
}

// Function to show detailed menu creation wizard
static void showMenuCreationWizard(CustomMenu& menu) {
    clear();
    refresh();
    
    mvprintw(1, 0, "=== Menu Creation Wizard ===");
    mvprintw(3, 0, "This wizard will guide you through creating a custom menu.");
    mvprintw(4, 0, "Your custom menu will be placed under: %s -> Custom User's Menu", menu.parentMenu.c_str());
    mvprintw(6, 0, "Menu Details:");
    mvprintw(7, 2, "Name: %s", menu.name.c_str());
    mvprintw(8, 2, "Parent Location: %s", menu.parentMenu.c_str());
    mvprintw(9, 2, "Full Path: Main Menu -> %s -> Custom User's Menu -> %s", 
              menu.parentMenu.c_str(), menu.name.c_str());
    
    mvprintw(11, 0, "What kind of commands will this menu contain?");
    mvprintw(12, 0, "Examples:");
    mvprintw(13, 2, "- System monitoring tools (htop, iotop, nethogs)");
    mvprintw(14, 2, "- Development tools (git commands, build scripts)");
    mvprintw(15, 2, "- File management utilities (custom find commands, backup scripts)");
    mvprintw(16, 2, "- Network tools (ping variants, curl commands)");
    
    mvprintw(18, 0, "Press any key to continue and start adding commands...");
    refresh();
    getch();
}

// Function to show command list management
static void manageCommandList(CustomMenu& menu) {
    while (true) {
        clear();
        refresh();
        
        mvprintw(1, 0, "=== Command List for: %s ===", menu.name.c_str());
        mvprintw(2, 0, "Parent: %s -> Custom User's Menu", menu.parentMenu.c_str());
        mvprintw(4, 0, "Current Commands (%zu):", menu.commands.size());
        
        // List current commands with details
        for (size_t i = 0; i < menu.commands.size(); i++) {
            const CustomCommand& cmd = menu.commands[i];
            mvprintw(5 + i, 2, "%zu. %s (%s) - %s", i + 1, cmd.name.c_str(), 
                     cmd.executionType.c_str(), cmd.baseCommand.c_str());
        }
        
        int menuRow = 6 + menu.commands.size();
        mvprintw(menuRow++, 0, "Options:");
        mvprintw(menuRow++, 2, "1. Add New Command");
        if (!menu.commands.empty()) {
            mvprintw(menuRow++, 2, "2. Edit Command (by number)");
            mvprintw(menuRow++, 2, "3. Remove Command (by number)");
            mvprintw(menuRow++, 2, "4. Preview Menu");
        }
        mvprintw(menuRow++, 2, "5. Save and Finish");
        mvprintw(menuRow++, 2, "6. Cancel and Discard");
        mvprintw(menuRow + 1, 0, "Choice: ");
        
        int ch = getch();
        
        if (ch == '1') {
            CustomCommand cmd = createCustomCommand();
            if (!cmd.name.empty() && !cmd.baseCommand.empty()) {
                menu.commands.push_back(cmd);
                
                // If this is the first command, use its name + " Menu" as the menu name
                if (menu.name.empty()) {
                    menu.name = cmd.name + " Menu";
                }
                
                clear();
                mvprintw(1, 0, "Command '%s' added successfully!", cmd.name.c_str());
                if (menu.commands.size() == 1) {
                    mvprintw(3, 0, "Menu name set to: '%s'", menu.name.c_str());
                    mvprintw(4, 0, "(Based on first command name)");
                }
                mvprintw(6, 0, "Press any key to continue...");
                refresh();
                getch();
            }
        } else if (ch == '2' && !menu.commands.empty()) {
            std::string indexStr = getUserInput("Enter command number to edit (1-" + std::to_string(menu.commands.size()) + "):", "1");
            if (!indexStr.empty()) {
                try {
                    int index = std::stoi(indexStr) - 1;
                    if (index >= 0 && index < (int)menu.commands.size()) {
                        // For simplicity, we'll recreate the command
                        CustomCommand newCmd = createCustomCommand();
                        if (!newCmd.name.empty() && !newCmd.baseCommand.empty()) {
                            menu.commands[index] = newCmd;
                        }
                    }
                } catch (...) {
                    // Invalid input, ignore
                }
            }
        } else if (ch == '3' && !menu.commands.empty()) {
            std::string indexStr = getUserInput("Enter command number to remove (1-" + std::to_string(menu.commands.size()) + "):", "1");
            if (!indexStr.empty()) {
                try {
                    int index = std::stoi(indexStr) - 1;
                    if (index >= 0 && index < (int)menu.commands.size()) {
                        std::string cmdName = menu.commands[index].name;
                        menu.commands.erase(menu.commands.begin() + index);
                        clear();
                        mvprintw(1, 0, "Command '%s' removed successfully!", cmdName.c_str());
                        mvprintw(2, 0, "Press any key to continue...");
                        refresh();
                        getch();
                    }
                } catch (...) {
                    // Invalid input, ignore
                }
            }
        } else if (ch == '4' && !menu.commands.empty()) {
            // Preview the menu
            clear();
            refresh();
            mvprintw(1, 0, "=== Menu Preview: %s ===", menu.name.c_str());
            mvprintw(2, 0, "Location: Main Menu -> %s -> Custom User's Menu -> %s", 
                     menu.parentMenu.c_str(), menu.name.c_str());
            mvprintw(4, 0, "Commands that will appear:");
            for (size_t i = 0; i < menu.commands.size(); i++) {
                mvprintw(5 + i, 2, "- %s", menu.commands[i].name.c_str());
            }
            mvprintw(6 + menu.commands.size(), 2, "- Back");
            mvprintw(8 + menu.commands.size(), 0, "Press any key to continue...");
            refresh();
            getch();
        } else if (ch == '5') {
            if (menu.commands.empty()) {
                clear();
                mvprintw(1, 0, "Error: Menu must have at least one command!");
                mvprintw(2, 0, "Press any key to continue...");
                refresh();
                getch();
                continue;
            }
            return; // Save and finish
        } else if (ch == '6') {
            // Confirm cancellation
            clear();
            mvprintw(1, 0, "Are you sure you want to cancel and discard this menu?");
            mvprintw(2, 0, "Menu: %s with %zu commands", menu.name.c_str(), menu.commands.size());
            mvprintw(4, 0, "y/Y - Yes, discard");
            mvprintw(5, 0, "Any other key - No, continue editing");
            mvprintw(7, 0, "Choice: ");
            refresh();
            
            int confirmCh = getch();
            if (confirmCh == 'y' || confirmCh == 'Y') {
                menu.commands.clear();
                return;
            }
        }
    }
}

// Enhanced function to add custom menu
static void addCustomMenu() {
    CustomMenu newMenu;
    
    // Select parent menu with better descriptions
    // Create a mapping to handle menu sorting correctly
    std::map<std::string, std::string> parentMapping = {
        {"Administration (system admin tools, package management)", "Administration"},
        {"System Information (monitoring, system details)", "System Information"},
        {"System Setting (configuration, input devices)", "System Setting"},
        {"Utilities (file management, general tools)", "Utilities"},
        {"Internet (network tools, web utilities)", "Internet"},
        {"Illegal Operation (system modification tools)", "Illegal Operation"}
    };
    
    // Create display options for the menu
    std::vector<std::string> parentMenus;
    for (const auto& pair : parentMapping) {
        parentMenus.push_back(pair.first);
    }
    
    clear();
    refresh();
    mvprintw(1, 0, "=== Select Parent Menu ===");
    mvprintw(3, 0, "Your custom menu will appear as:");
    mvprintw(4, 0, "Main Menu -> [Selected Parent] -> Custom User's Menu -> [Menu Name]");
    mvprintw(6, 0, "Choose the most appropriate parent category:");
    
    Menu parentMenu("=== Select Parent Menu ===", parentMenus);
    int parentChoice = parentMenu.run();
    if (parentChoice == -1 || parentChoice >= (int)parentMenus.size()) return; // Cancelled
    
    // IMPORTANT: Get the actual selected text from the SORTED menu options
    // The Menu class sorts options, so parentMenus[parentChoice] is wrong
    // We need to get the actual selected option from the menu after sorting
    std::string selectedDisplayOption = parentMenu.getOption(parentChoice);
    
    // Now find the corresponding key
    auto it = parentMapping.find(selectedDisplayOption);
    if (it != parentMapping.end()) {
        newMenu.parentMenu = it->second;
    } else {
        // Fallback - this shouldn't happen
        newMenu.parentMenu = "Administration";
    }
    
    // Show menu creation wizard
    showMenuCreationWizard(newMenu);
    
    // Manage commands
    manageCommandList(newMenu);
    
    // Save if not cancelled (commands not empty)
    if (!newMenu.commands.empty()) {
        if (saveCustomMenu(newMenu)) {
            customMenus.push_back(newMenu);
            clear();
            mvprintw(1, 0, "=== Menu Created Successfully! ===");
            mvprintw(3, 0, "Menu Name: %s", newMenu.name.c_str());
            mvprintw(4, 0, "Location: Main Menu -> %s -> Custom User's Menu -> %s", 
                     newMenu.parentMenu.c_str(), newMenu.name.c_str());
            mvprintw(5, 0, "Commands: %zu", newMenu.commands.size());
            mvprintw(7, 0, "You can now access your custom menu from the main menu!");
            mvprintw(9, 0, "Press any key to continue...");
            refresh();
            getch();
        } else {
            clear();
            mvprintw(1, 0, "Error: Failed to save menu!");
            mvprintw(2, 0, "Please check your permissions and disk space.");
            mvprintw(3, 0, "Press any key to continue...");
            refresh();
            getch();
        }
    }
}

// Function to delete custom menu
static void deleteCustomMenu() {
    loadAllCustomMenus();
    
    if (customMenus.empty()) {
        clear();
        mvprintw(1, 0, "No custom menus found to delete.");
        mvprintw(2, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Show list of menus to delete
    std::vector<std::string> menuNames;
    for (const CustomMenu& menu : customMenus) {
        menuNames.push_back(menu.name + " (" + menu.parentMenu + ", " + std::to_string(menu.commands.size()) + " commands)");
    }
    
    Menu deleteMenu("=== Select Menu to Delete ===", menuNames);
    int choice = deleteMenu.run();
    
    if (choice == -1 || choice >= (int)customMenus.size()) {
        return; // Cancelled
    }
    
    const CustomMenu& selectedMenu = customMenus[choice];
    
    // Confirmation
    clear();
    refresh();
    mvprintw(1, 0, "=== Confirm Menu Deletion ===");
    mvprintw(3, 0, "You are about to delete:");
    mvprintw(4, 2, "Menu: %s", selectedMenu.name.c_str());
    mvprintw(5, 2, "Parent: %s", selectedMenu.parentMenu.c_str());
    mvprintw(6, 2, "Commands: %zu", selectedMenu.commands.size());
    mvprintw(8, 0, "This action cannot be undone!");
    mvprintw(10, 0, "Type 'DELETE' to confirm (case sensitive): ");
    refresh();
    
    std::string confirmation = getUserInput("", "DELETE");
    
    if (confirmation == "DELETE") {
        // Delete the menu directory
        std::string menuDir = getCustomMenusDirPath() + "/" + selectedMenu.name;
        std::string rmCommand = "rm -rf '" + menuDir + "'";
        
        if (system(rmCommand.c_str()) == 0) {
            // Remove from memory
            customMenus.erase(customMenus.begin() + choice);
            
            clear();
            mvprintw(1, 0, "Menu '%s' deleted successfully!", selectedMenu.name.c_str());
            mvprintw(2, 0, "Press any key to continue...");
            refresh();
            getch();
        } else {
            clear();
            mvprintw(1, 0, "Error: Failed to delete menu!");
            mvprintw(2, 0, "Check file permissions and try again.");
            mvprintw(3, 0, "Press any key to continue...");
            refresh();
            getch();
        }
    } else {
        clear();
        mvprintw(1, 0, "Menu deletion cancelled.");
        mvprintw(2, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}

// Function to show detailed menu information
static void showDetailedMenuList() {
    loadAllCustomMenus();
    
    if (customMenus.empty()) {
        clear();
        mvprintw(1, 0, "=== No Custom Menus Found ===");
        mvprintw(3, 0, "You haven't created any custom menus yet.");
        mvprintw(4, 0, "Use 'Add Menu' to create your first custom menu.");
        mvprintw(6, 0, "Custom menus allow you to:");
        mvprintw(7, 2, "- Create shortcuts for frequently used commands");
        mvprintw(8, 2, "- Build complex command combinations");
        mvprintw(9, 2, "- Organize tools by category");
        mvprintw(10, 2, "- Share command sets across systems");
        mvprintw(12, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Group menus by parent
    std::map<std::string, std::vector<CustomMenu*>> menusByParent;
    for (CustomMenu& menu : customMenus) {
        menusByParent[menu.parentMenu].push_back(&menu);
    }
    
    clear();
    refresh();
    mvprintw(1, 0, "=== Detailed Custom Menu List ===");
    mvprintw(2, 0, "Total Menus: %zu", customMenus.size());
    
    int currentRow = 4;
    for (const auto& pair : menusByParent) {
        mvprintw(currentRow++, 0, "[%s]", pair.first.c_str());
        
        for (const CustomMenu* menu : pair.second) {
            mvprintw(currentRow++, 2, "• %s (%zu commands)", 
                     menu->name.c_str(), menu->commands.size());
            
            // Show first few commands
            for (size_t i = 0; i < menu->commands.size() && i < 3; i++) {
                mvprintw(currentRow++, 4, "- %s (%s)", 
                         menu->commands[i].name.c_str(),
                         menu->commands[i].executionType.c_str());
            }
            if (menu->commands.size() > 3) {
                mvprintw(currentRow++, 4, "... and %zu more commands", 
                         menu->commands.size() - 3);
            }
            currentRow++; // Empty line between menus
        }
    }
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

// Enhanced main menu management function
void showMenuManagementMenu() {
    while (true) {
        loadAllCustomMenus(); // Refresh the list
        
        std::vector<std::string> options = {
            "Add Menu",
            "List & View Menus",
            "Delete Menu",
            "Back"
        };
        
        Menu menu("=== Manage User's Menu ===", options);
        int choice = menu.run();
        
        if (choice == -1) { // ESC
            break;
        }
        
        // Get the actual selected option to avoid sorting issues
        std::string selectedOption = menu.getOption(choice);
        
        if (selectedOption == "Add Menu") {
            addCustomMenu();
        } else if (selectedOption == "List & View Menus") {
            showDetailedMenuList();
        } else if (selectedOption == "Delete Menu") {
            deleteCustomMenu();
        } else if (selectedOption == "Back") {
            break;
        }
    }
}

// Function to get custom menus for a specific parent
std::vector<std::string> getCustomMenusForParent(const std::string& parentMenu) {
    std::vector<std::string> menus;
    
    for (const CustomMenu& menu : customMenus) {
        if (menu.parentMenu == parentMenu) {
            menus.push_back(menu.name);
        }
    }
    
    return menus;
}

// Function to execute a custom menu
void executeCustomMenu(const std::string& menuName) {
    CustomMenu* targetMenu = nullptr;
    
    // Find the menu
    for (CustomMenu& menu : customMenus) {
        if (menu.name == menuName) {
            targetMenu = &menu;
            break;
        }
    }
    
    if (targetMenu == nullptr) {
        return; // Menu not found
    }
    
    // Create options from commands
    std::vector<std::string> options;
    for (const CustomCommand& cmd : targetMenu->commands) {
        options.push_back(cmd.name);
    }
    options.push_back("Back");
    
    Menu customMenu("=== " + menuName + " ===", options);
    
    while (true) {
        int choice = customMenu.run();
        
        if (choice == -1 || choice == (int)targetMenu->commands.size()) { // Back or ESC
            break;
        } else if (choice >= 0 && choice < (int)targetMenu->commands.size()) {
            executeCustomCommand(targetMenu->commands[choice]);
        }
    }
}

// Function to initialize custom menus at startup
void initializeCustomMenus() {
    loadAllCustomMenus();
}
