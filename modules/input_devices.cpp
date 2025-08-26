// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// Input devices module for managing xinput devices

#include <ncurses.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "../menu.hpp"
#include "input_devices.hpp"

using namespace std;

void showInputDevicesMenu() {
    std::vector<std::string> options = {
        "Input Device List",
        "Detach Input Device",
        "Enable/Disable Input Device",
        "Reattach Input Device",
        "Test Input Device",
        "Back"
    };
    
    Menu menu("=== Input Devices (xinput) ===", options);
    
    // Function mapping for input devices menu
    std::map<std::string, std::function<void()>> inputDevicesFunctions = {
        {"Detach Input Device", showDetachInputDevice},
        {"Enable/Disable Input Device", showEnableDisableInputDevice},
        {"Input Device List", showInputDeviceList},
        {"Reattach Input Device", showReattachInputDevice},
        {"Test Input Device", showTestInputDevice}
    };
    
    menu.executeMenu(inputDevicesFunctions);
}

void showInputDeviceList() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Input Device List (xinput) ===");
    
    // Capture xinput output and display it within ncurses
    FILE* pipe = popen("xinput", "r");
    if (pipe) {
        char buffer[256];
        int line = 2;
        
        while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 3) {
            std::string text = buffer;
            // Remove trailing newline
            if (!text.empty() && text.back() == '\n') text.pop_back();
            
            // Truncate to fit screen width
            if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
            
            mvprintw(line, 0, "%s", text.c_str());
            line++;
        }
        pclose(pipe);
    }
    
    mvprintw(maxLines - 2, 0, "Press any key to continue...");
    refresh();
    getch();
}

void showDetachInputDevice() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Detach Input Device ===");
    mvprintw(2, 0, "This will detach an input device using xinput float <id>");
    mvprintw(3, 0, "");
    mvprintw(4, 0, "Enter the device ID number (or press ESC to go back):");
    mvprintw(5, 0, "> ");
    refresh();
    
    // Get user input for device ID with proper ESC handling
    std::string deviceId = "";
    int ch;
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!deviceId.empty()) {
                deviceId.pop_back();
                mvprintw(5, 2 + deviceId.length(), " "); // Clear the character
                move(5, 2 + deviceId.length()); // Move cursor back
                refresh();
            }
            continue;
        }
        
        // Only accept numeric characters
        if (ch >= '0' && ch <= '9') {
            deviceId += (char)ch;
            mvprintw(5, 2 + deviceId.length() - 1, "%c", ch);
            refresh();
        }
    }
    
    // Validate input
    if (deviceId.empty()) {
        clear();
        mvprintw(0, 0, "=== No Input ===");
        mvprintw(2, 0, "No device ID entered.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Show confirmation
    clear();
    mvprintw(0, 0, "=== Confirm Detach ===");
    mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
    mvprintw(3, 0, "Command: xinput float %s", deviceId.c_str());
    mvprintw(4, 0, "");
    mvprintw(5, 0, "Are you sure you want to detach this device?");
    mvprintw(6, 0, "Press 'y' to confirm or any other key to cancel...");
    refresh();
    
    ch = getch();
    if (ch == 'y' || ch == 'Y') {
        // Execute the detach command
        clear();
        mvprintw(0, 0, "=== Detaching Device ===");
        mvprintw(2, 0, "Executing: xinput float %s", deviceId.c_str());
        refresh();
        
        // Execute xinput command within ncurses
        std::string command = "xinput float " + deviceId;
        int result = system(command.c_str());
        
        // Show result
        clear();
        if (result == 0) {
            mvprintw(0, 0, "=== Device Detached Successfully ===");
            mvprintw(2, 0, "Device ID %s has been detached.", deviceId.c_str());
        } else {
            mvprintw(0, 0, "=== Error Detaching Device ===");
            mvprintw(2, 0, "Failed to detach device ID %s.", deviceId.c_str());
            mvprintw(3, 0, "Please check if the device ID is correct.");
        }
        mvprintw(4, 0, "");
        mvprintw(5, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}

void showEnableDisableInputDevice() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Enable/Disable Input Device ===");
    mvprintw(2, 0, "This will enable or disable an input device using xinput enable/disable <id>");
    mvprintw(3, 0, "");
    mvprintw(4, 0, "Enter the device ID number (or press ESC to go back):");
    mvprintw(5, 0, "> ");
    refresh();
    
    // Get user input for device ID with proper ESC handling
    std::string deviceId = "";
    int ch;
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!deviceId.empty()) {
                deviceId.pop_back();
                mvprintw(5, 2 + deviceId.length(), " "); // Clear the character
                move(5, 2 + deviceId.length()); // Move cursor back
                refresh();
            }
            continue;
        }
        
        // Only accept numeric characters
        if (ch >= '0' && ch <= '9') {
            deviceId += (char)ch;
            mvprintw(5, 2 + deviceId.length() - 1, "%c", ch);
            refresh();
        }
    }
    
    // Validate device ID input
    if (deviceId.empty()) {
        clear();
        mvprintw(0, 0, "=== No Input ===");
        mvprintw(2, 0, "No device ID entered.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Show enable/disable options
    clear();
    mvprintw(0, 0, "=== Enable/Disable Input Device ===");
    mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
    mvprintw(3, 0, "");
    mvprintw(4, 0, "Select action:");
    mvprintw(5, 0, "1. Enable device");
    mvprintw(6, 0, "2. Disable device");
    mvprintw(7, 0, "");
    mvprintw(8, 0, "Press 1, 2, or ESC to go back...");
    refresh();
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for '1' (Enable)
        if (ch == '1') {
            // Show confirmation for enable
            clear();
            mvprintw(0, 0, "=== Confirm Enable ===");
            mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
            mvprintw(3, 0, "Command: xinput enable %s", deviceId.c_str());
            mvprintw(4, 0, "");
            mvprintw(5, 0, "Are you sure you want to enable this device?");
            mvprintw(6, 0, "Press 'y' to confirm or any other key to cancel...");
            refresh();
            
            ch = getch();
            if (ch == 'y' || ch == 'Y') {
                // Execute the enable command
                clear();
                mvprintw(0, 0, "=== Enabling Device ===");
                mvprintw(2, 0, "Executing: xinput enable %s", deviceId.c_str());
                refresh();
                
                // Execute xinput command within ncurses
                std::string command = "xinput enable " + deviceId;
                int result = system(command.c_str());
                
                // Show result
                clear();
                if (result == 0) {
                    mvprintw(0, 0, "=== Device Enabled Successfully ===");
                    mvprintw(2, 0, "Device ID %s has been enabled.", deviceId.c_str());
                } else {
                    mvprintw(0, 0, "=== Error Enabling Device ===");
                    mvprintw(2, 0, "Failed to enable device ID %s.", deviceId.c_str());
                    mvprintw(3, 0, "Please check if the device ID is correct.");
                }
                mvprintw(4, 0, "");
                mvprintw(5, 0, "Press any key to continue...");
                refresh();
                getch();
            }
            return;
        }
        
        // Check for '2' (Disable)
        if (ch == '2') {
            // Show confirmation for disable
            clear();
            mvprintw(0, 0, "=== Confirm Disable ===");
            mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
            mvprintw(3, 0, "Command: xinput disable %s", deviceId.c_str());
            mvprintw(4, 0, "");
            mvprintw(5, 0, "Are you sure you want to disable this device?");
            mvprintw(6, 0, "Press 'y' to confirm or any other key to cancel...");
            refresh();
            
            ch = getch();
            if (ch == 'y' || ch == 'Y') {
                // Execute the disable command
                clear();
                mvprintw(0, 0, "=== Disabling Device ===");
                mvprintw(2, 0, "Executing: xinput disable %s", deviceId.c_str());
                refresh();
                
                // Execute xinput command within ncurses
                std::string command = "xinput disable " + deviceId;
                int result = system(command.c_str());
                
                // Show result
                clear();
                if (result == 0) {
                    mvprintw(0, 0, "=== Device Disabled Successfully ===");
                    mvprintw(2, 0, "Device ID %s has been disabled.", deviceId.c_str());
                } else {
                    mvprintw(0, 0, "=== Error Disabling Device ===");
                    mvprintw(2, 0, "Failed to disable device ID %s.", deviceId.c_str());
                    mvprintw(3, 0, "Please check if the device ID is correct.");
                }
                mvprintw(4, 0, "");
                mvprintw(5, 0, "Press any key to continue...");
                refresh();
                getch();
            }
            return;
        }
    }
}

void showTestInputDevice() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Test Input Device ===");
    mvprintw(2, 0, "This will test an input device using xinput test <id>");
    mvprintw(3, 0, "The test will show real-time input events from the device.");
    mvprintw(4, 0, "");
    mvprintw(5, 0, "Enter the device ID number (or press ESC to go back):");
    mvprintw(6, 0, "> ");
    refresh();
    
    // Get user input for device ID with proper ESC handling
    std::string deviceId = "";
    int ch;
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!deviceId.empty()) {
                deviceId.pop_back();
                mvprintw(6, 2 + deviceId.length(), " "); // Clear the character
                move(6, 2 + deviceId.length()); // Move cursor back
                refresh();
            }
            continue;
        }
        
        // Only accept numeric characters
        if (ch >= '0' && ch <= '9') {
            deviceId += (char)ch;
            mvprintw(6, 2 + deviceId.length() - 1, "%c", ch);
            refresh();
        }
    }
    
    // Validate device ID input
    if (deviceId.empty()) {
        clear();
        mvprintw(0, 0, "=== No Input ===");
        mvprintw(2, 0, "No device ID entered.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Show confirmation
    clear();
    mvprintw(0, 0, "=== Confirm Test ===");
    mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
    mvprintw(3, 0, "Command: xinput test %s", deviceId.c_str());
    mvprintw(4, 0, "");
    mvprintw(5, 0, "This will start testing the input device.");
    mvprintw(6, 0, "Press any key on the device to see events.");
    mvprintw(7, 0, "Press Ctrl+C in the test window to stop.");
    mvprintw(8, 0, "");
    mvprintw(9, 0, "Press 'y' to start testing or any other key to cancel...");
    refresh();
    
    ch = getch();
    if (ch == 'y' || ch == 'Y') {
        // Execute the test command
        clear();
        mvprintw(0, 0, "=== Testing Input Device ===");
        mvprintw(2, 0, "Executing: xinput test %s", deviceId.c_str());
        mvprintw(3, 0, "");
        mvprintw(4, 0, "A new terminal window will open for testing.");
        mvprintw(5, 0, "Press any key on the device to see events.");
        mvprintw(6, 0, "Press Ctrl+C in the test window to stop.");
        mvprintw(7, 0, "");
        mvprintw(8, 0, "Press any key to continue...");
        refresh();
        getch();
        
        // Execute xinput test command in a new terminal window
        std::string command = "xterm -title \"Input Device Test - Device " + deviceId + "\" -e \"xinput test " + deviceId + "; echo 'Press any key to close...'; read\"";
        system(command.c_str());
        
        // Show completion message
        clear();
        mvprintw(0, 0, "=== Test Completed ===");
        mvprintw(2, 0, "The input device test has been completed.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}

void showReattachInputDevice() {
    clear();
    refresh();
    
    int maxLines, maxCols;
    getmaxyx(stdscr, maxLines, maxCols);
    
    mvprintw(0, 0, "=== Reattach Input Device ===");
    mvprintw(2, 0, "This will reattach an input device using xinput reattach <id> <master>");
    mvprintw(3, 0, "");
    mvprintw(4, 0, "Enter the device ID number (or press ESC to go back):");
    mvprintw(5, 0, "> ");
    refresh();
    
    // Get user input for device ID with proper ESC handling
    std::string deviceId = "";
    int ch;
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!deviceId.empty()) {
                deviceId.pop_back();
                mvprintw(5, 2 + deviceId.length(), " "); // Clear the character
                move(5, 2 + deviceId.length()); // Move cursor back
                refresh();
            }
            continue;
        }
        
        // Only accept numeric characters
        if (ch >= '0' && ch <= '9') {
            deviceId += (char)ch;
            mvprintw(5, 2 + deviceId.length() - 1, "%c", ch);
            refresh();
        }
    }
    
    // Validate device ID input
    if (deviceId.empty()) {
        clear();
        mvprintw(0, 0, "=== No Input ===");
        mvprintw(2, 0, "No device ID entered.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Get master ID input
    clear();
    mvprintw(0, 0, "=== Reattach Input Device ===");
    mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
    mvprintw(3, 0, "");
    mvprintw(4, 0, "Enter the master ID number (or press ESC to go back):");
    mvprintw(5, 0, "> ");
    refresh();
    
    // Get user input for master ID with proper ESC handling
    std::string masterId = "";
    
    while (true) {
        ch = getch();
        
        // Check for ESC key (27)
        if (ch == 27) {
            return; // Go back to previous menu
        }
        
        // Check for Enter key (10 or 13)
        if (ch == 10 || ch == 13) {
            break; // End input
        }
        
        // Check for backspace (8 or 127)
        if (ch == 8 || ch == 127) {
            if (!masterId.empty()) {
                masterId.pop_back();
                mvprintw(5, 2 + masterId.length(), " "); // Clear the character
                move(5, 2 + masterId.length()); // Move cursor back
                refresh();
            }
            continue;
        }
        
        // Only accept numeric characters
        if (ch >= '0' && ch <= '9') {
            masterId += (char)ch;
            mvprintw(5, 2 + masterId.length() - 1, "%c", ch);
            refresh();
        }
    }
    
    // Validate master ID input
    if (masterId.empty()) {
        clear();
        mvprintw(0, 0, "=== No Input ===");
        mvprintw(2, 0, "No master ID entered.");
        mvprintw(3, 0, "");
        mvprintw(4, 0, "Press any key to continue...");
        refresh();
        getch();
        return;
    }
    
    // Show confirmation
    clear();
    mvprintw(0, 0, "=== Confirm Reattach ===");
    mvprintw(2, 0, "Device ID: %s", deviceId.c_str());
    mvprintw(3, 0, "Master ID: %s", masterId.c_str());
    mvprintw(4, 0, "Command: xinput reattach %s %s", deviceId.c_str(), masterId.c_str());
    mvprintw(5, 0, "");
    mvprintw(6, 0, "Are you sure you want to reattach this device?");
    mvprintw(7, 0, "Press 'y' to confirm or any other key to cancel...");
    refresh();
    
    ch = getch();
    if (ch == 'y' || ch == 'Y') {
        // Execute the reattach command
        clear();
        mvprintw(0, 0, "=== Reattaching Device ===");
        mvprintw(2, 0, "Executing: xinput reattach %s %s", deviceId.c_str(), masterId.c_str());
        refresh();
        
        // Execute xinput command within ncurses
        std::string command = "xinput reattach " + deviceId + " " + masterId;
        int result = system(command.c_str());
        
        // Show result
        clear();
        if (result == 0) {
            mvprintw(0, 0, "=== Device Reattached Successfully ===");
            mvprintw(2, 0, "Device ID %s has been reattached to master %s.", deviceId.c_str(), masterId.c_str());
        } else {
            mvprintw(0, 0, "=== Error Reattaching Device ===");
            mvprintw(2, 0, "Failed to reattach device ID %s to master %s.", deviceId.c_str(), masterId.c_str());
            mvprintw(3, 0, "Please check if the device ID and master ID are correct.");
        }
        mvprintw(4, 0, "");
        mvprintw(5, 0, "Press any key to continue...");
        refresh();
        getch();
    }
}
