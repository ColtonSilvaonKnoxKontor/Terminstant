// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// Screenfetch is kinda problematic with ncurses, so it just outputs without
// any color except white.

#include <ncurses.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "../menu.hpp"

using namespace std;

void showScreenfetchMenu() {
    std::vector<std::string> options = {
        "Launch screenfetch",
        "Back"
    };
    
    Menu menu("=== Screenfetch ===", options);
    
    int choice;
    while ((choice = menu.run()) != -1) {
        switch (choice) {
            case 0: { // Launch screenfetch
                // Display screenfetch output within ncurses
                clear();
                refresh();
                
                // Get terminal dimensions
                int maxLines, maxCols;
                getmaxyx(stdscr, maxLines, maxCols);
                
                // Capture screenfetch output
                FILE* pipe = popen("screenfetch", "r");
                if (pipe) {
                    char buffer[256];
                    int line = 0;
                    
                    while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
                        std::string cleanLine = buffer;
                        
                        // Strip ANSI escape codes
                        size_t pos = 0;
                        while ((pos = cleanLine.find("\033[")) != std::string::npos) {
                            size_t end = cleanLine.find('m', pos);
                            if (end != std::string::npos) {
                                std::string code = cleanLine.substr(pos + 2, end - pos - 2);
                                
                                // Only remove color-related codes, keep positioning/formatting
                                if (code.find("0") != std::string::npos ||      // Reset
                                    code.find("1") != std::string::npos ||      // Bold
                                    code.find("31") != std::string::npos ||     // Red
                                    code.find("32") != std::string::npos ||     // Green
                                    code.find("33") != std::string::npos ||     // Yellow
                                    code.find("34") != std::string::npos ||     // Blue
                                    code.find("35") != std::string::npos ||     // Magenta
                                    code.find("36") != std::string::npos ||     // Cyan
                                    code.find("37") != std::string::npos) {     // White
                                    
                                    cleanLine.erase(pos, end - pos + 1);
                                } else {
                                    // Skip non-color codes (like positioning codes)
                                    pos = end + 1;
                                }
                            } else {
                                break;
                            }
                        }
                        
                        // Remove any remaining control characters
                        cleanLine.erase(std::remove(cleanLine.begin(), cleanLine.end(), '\r'), cleanLine.end());
                        
                        // Display the line
                        if (!cleanLine.empty() && cleanLine != "\n") {
                            mvprintw(line, 0, "%s", cleanLine.c_str());
                            line++;
                        }
                    }
                    pclose(pipe);
                }
                
                mvprintw(maxLines - 1, 0, "Press any key to continue...");
                refresh();
                getch();
                break;
            }
            case 1: // Back
                return;
        }
    }
}
