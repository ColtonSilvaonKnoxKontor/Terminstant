// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// ls module for file and folder listing functionality

#include <ncursesw/ncurses.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <map>
#include <functional>
#include <unistd.h>
#include <cstdlib>
#include "../menu.hpp"

using namespace std;

static void renderLsOutput() {
    showScrollableOutput("ls --color=never", "=== List Files and Folders ===");
}

static void renderLsDetailedOutput() {
    showScrollableOutput("ls -lh --color=never", "=== Detailed Lists ===");
}

static void renderLsRecursiveOutput() {
    showScrollableOutput("ls -R --color=never", "=== Recursive Listing ===");
}

static void renderLsHiddenOutput() {
    showScrollableOutput("ls -a --color=never", "=== Show Hidden Files ===");
}

static void changeDirectory() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    mvprintw(3, 0, "Enter new directory path (or press ESC to cancel):");
    mvprintw(4, 0, "> ");
    
    // Enable echo and cursor for input
    echo();
    curs_set(1);
    
    char input[1024];
    int ch;
    int pos = 0;
    
    while ((ch = getch()) != 27 && ch != 10 && pos < 1023) { // ESC or Enter
        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (pos > 0) {
                pos--;
                mvprintw(4, 2 + pos, " ");
                move(4, 2 + pos);
            }
        } else if (ch >= 32 && ch <= 126) { // Printable characters
            input[pos] = ch;
            mvaddch(4, 2 + pos, ch);
            pos++;
        }
        refresh();
    }
    
    input[pos] = '\0';
    
    // Disable echo and cursor
    noecho();
    curs_set(0);
    
    if (ch == 10 && pos > 0) { // Enter pressed and input not empty
        std::string newDir(input);
        
        // Try to change directory
        if (chdir(newDir.c_str()) == 0) {
            // Success - update our tracking
            char* cwd = getcwd(nullptr, 0);
            if (cwd != nullptr) {
                updateCurrentDirectory(std::string(cwd));
                free(cwd);
            }
            
            // Show success message
            if (has_colors()) attron(COLOR_PAIR(5)); // Green
            mvprintw(6, 0, "Directory changed successfully to: %s", newDir.c_str());
            if (has_colors()) attroff(COLOR_PAIR(5));
        } else {
            // Error - show message
            if (has_colors()) attron(COLOR_PAIR(4)); // Red
            mvprintw(6, 0, "Error: Cannot change to directory '%s'", newDir.c_str());
            if (has_colors()) attroff(COLOR_PAIR(4));
        }
        
        mvprintw(maxLines - 1, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}

static void exitSubdirectory() {
    clear();
    refresh();
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    // Display current directory with color
    std::string currentDir = getCurrentDirectory();
    if (has_colors()) attron(COLOR_PAIR(5)); // Green color
    mvprintw(1, 0, "Current Directory: %s", currentDir.c_str());
    if (has_colors()) attroff(COLOR_PAIR(5));
    
    // Try to change to parent directory
    if (chdir("..") == 0) {
        // Success - update our tracking
        char* cwd = getcwd(nullptr, 0);
        if (cwd != nullptr) {
            updateCurrentDirectory(std::string(cwd));
            std::string newDir(cwd);
            free(cwd);
            
            // Show success message
            if (has_colors()) attron(COLOR_PAIR(5)); // Green
            mvprintw(3, 0, "Changed to parent directory: %s", newDir.c_str());
            if (has_colors()) attroff(COLOR_PAIR(5));
        }
    } else {
        // Error - show message
        if (has_colors()) attron(COLOR_PAIR(4)); // Red
        mvprintw(3, 0, "Error: Cannot change to parent directory");
        if (has_colors()) attroff(COLOR_PAIR(4));
    }
    
    mvprintw(maxLines - 1, 0, "Press any key to continue...");
    refresh();
    getch();
}

// Helper function to get user input for multiple files/folders
static std::string getMultipleInputs(const std::string& prompt, const std::string& example) {
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
    mvprintw(4, 0, "Example: %s", example.c_str());
    mvprintw(5, 0, "(Press ESC to cancel)");
    mvprintw(7, 0, "> ");
    
    // Enable echo and cursor for input
    echo();
    curs_set(1);
    
    char input[2048];
    int ch;
    int pos = 0;
    
    while ((ch = getch()) != 27 && ch != 10 && pos < 2047) { // ESC or Enter
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

// Helper function to execute command with interactive prompts (for cp -i, mv -i)
static void executeCommandWithInteractiveInput(const std::string& command, const std::string& title) {
    executeInteractiveCommand(command, title);
}

// Helper function to execute command and show scrollable output (for non-interactive commands)
static void executeCommandWithScrollableOutput(const std::string& command, const std::string& title) {
    showScrollableOutput(command, title);
}

static void copyFileAndFolder() {
    std::string sources = getMultipleInputs(
        "Enter source file(s)/folder(s) (space separated):",
        "file1.txt file2.txt folder1/"
    );
    
    if (sources.empty()) return;
    
    std::string destination = getMultipleInputs(
        "Enter destination folder (leave empty if copying to current directory):",
        "backup/ or ../parent/ or /home/user/documents/"
    );
    
    // Build command
    std::string command = "cp -iv " + sources;
    if (!destination.empty()) {
        // Ensure destination ends with / if it's a directory
        if (destination.back() != '/') {
            destination += "/";
        }
        command += " " + destination;
    }
    
    executeCommandWithInteractiveInput(command, "=== Copy Files and Folders ===");
}

static void copyAllFilesInsideDirectory() {
    std::string source = getMultipleInputs(
        "Enter source directory:",
        "source_folder/ or /path/to/source/"
    );
    
    if (source.empty()) return;
    
    std::string destination = getMultipleInputs(
        "Enter destination directory:",
        "destination_folder/ or /path/to/destination/"
    );
    
    if (destination.empty()) return;
    
    // Ensure directories end with /
    if (source.back() != '/') source += "/";
    if (destination.back() != '/') destination += "/";
    
    std::string command = "cp -irv " + source + " " + destination;
    
    executeCommandWithInteractiveInput(command, "=== Copy All Files Inside Directory ===");
}

static void copyAllFilesPreserveAttributes() {
    std::string source = getMultipleInputs(
        "Enter source directory:",
        "source_folder/ or /path/to/source/"
    );
    
    if (source.empty()) return;
    
    std::string destination = getMultipleInputs(
        "Enter destination directory:",
        "destination_folder/ or /path/to/destination/"
    );
    
    if (destination.empty()) return;
    
    // Ensure directories end with /
    if (source.back() != '/') source += "/";
    if (destination.back() != '/') destination += "/";
    
    std::string command = "cp -iav " + source + " " + destination;
    
    executeCommandWithInteractiveInput(command, "=== Copy Files and Preserve Attributes ===");
}

static void moveOrRenameFileFolder() {
    std::string sources = getMultipleInputs(
        "Enter source file(s)/folder(s) to move/rename (space separated):",
        "file1.txt file2.txt folder1/ OR oldname.txt"
    );
    
    if (sources.empty()) return;
    
    std::string destination = getMultipleInputs(
        "Enter destination (folder for move, new name for rename, empty for current dir):",
        "backup/ OR newname.txt OR ../parent/ OR (empty)"
    );
    
    // Build command
    std::string command = "mv -iv " + sources;
    if (!destination.empty()) {
        command += " " + destination;
    }
    
    executeCommandWithInteractiveInput(command, "=== Move or Rename Files and Folders ===");
}

static void removeFiles() {
    std::string files = getMultipleInputs(
        "Enter file(s) to remove (space separated):",
        "file1.txt file2.txt document.pdf"
    );
    
    if (files.empty()) return;
    
    // Build command with interactive confirmation
    std::string command = "rm -iv " + files;
    
    executeCommandWithInteractiveInput(command, "=== Remove Files ===");
}

static void removeFolders() {
    std::string folders = getMultipleInputs(
        "Enter folder(s) to remove (space separated):",
        "folder1/ folder2/ old_directory/"
    );
    
    if (folders.empty()) return;
    
    // Build command with recursive, interactive, and verbose flags
    std::string command = "rm -riv " + folders;
    
    executeCommandWithInteractiveInput(command, "=== Remove Folders ===");
}

void showLsMenu() {
    vector<string> options = {
        "Change Directory",
        "Copy a File and Folder",
        "Copy All Files and Preserve Attributes",
        "Copy All Files Inside Directory",
        "Detailed Lists",
        "Exit Subdirectory", 
        "List Files and Folders", 
        "Move or Rename a File or Folder",
        "Recursive Listing",
        "Remove Files",
        "Remove Folders",
        "Show Hidden Files",
        "Back"
    };
    Menu menu("=== Manage Files and Folders ==", options);
    
    // Function mapping for ls menu - sorted alphabetically by option name
    std::map<std::string, std::function<void()>> lsFunctions = {
        {"Change Directory", changeDirectory},
        {"Copy a File and Folder", copyFileAndFolder},
        {"Copy All Files and Preserve Attributes", copyAllFilesPreserveAttributes},
        {"Copy All Files Inside Directory", copyAllFilesInsideDirectory},
        {"Detailed Lists", renderLsDetailedOutput},
        {"Exit Subdirectory", exitSubdirectory},
        {"List Files and Folders", renderLsOutput},
        {"Move or Rename a File or Folder", moveOrRenameFileFolder},
        {"Recursive Listing", renderLsRecursiveOutput},
        {"Remove Files", removeFiles},
        {"Remove Folders", removeFolders},
        {"Show Hidden Files", renderLsHiddenOutput}
    };
    
    menu.executeMenu(lsFunctions);
}
