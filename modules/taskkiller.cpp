// 2025 Colton Silva
//
// Part of Terminstant, a menu-driven tool for launching 
// text-based program without entering command line

#include <ncurses.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <map>
#include <functional>
#include "../menu.hpp"

using namespace std;

// Global flag to control the killer thread
static atomic<bool> killerActive(false);
static thread killerThread;

void monitorAndKillTaskManagers() {
    const vector<string> taskManagers = {
        // You may add a task manager here
        "htop", "btop", "top", "atop", "gtop",
        "vtop", "bashtop", "glances", "ksysguard", "gnome-system-monitor",
        "xfce4-taskmanager", "lxtask", "taskmgr", "resmon",
        "kSysGuard", "mate-system-monitor", "nmon", "bpytop", "conky",
        "perf", "iotop", "ps_mem", "nmon"
    };

    while (killerActive.load()) {
        for (const auto &proc : taskManagers) {
            string cmd = "pkill -9 -f \"" + proc + "\" > /dev/null 2>&1";
            system(cmd.c_str());
        }
        this_thread::sleep_for(chrono::milliseconds(500));
    }
}

void stopTaskKiller() {
    killerActive.store(false);
    if (killerThread.joinable()) {
        killerThread.join();
    }
}

void showTaskKillerMenu() {
    std::vector<std::string> options = {
        "Enable Task Manager Killer",
        "Disable Task Manager Killer",
        "Status",
        "Back"
    };
    
    Menu menu("=== Task Manager Killer ===", options);
    
    // Function mapping for task killer menu
    std::map<std::string, std::function<void()>> taskKillerFunctions = {
        {"Disable Task Manager Killer", []() {
            if (killerActive.load()) {
                stopTaskKiller();
                // Show confirmation
                clear();
                mvprintw(5, 5, "Task Manager Killer DISABLED!");
                mvprintw(6, 5, "Press any key to continue...");
                refresh();
                getch();
            }
        }},
        {"Enable Task Manager Killer", []() {
            if (!killerActive.load()) {
                killerActive.store(true);
                killerThread = thread(monitorAndKillTaskManagers);
                // Show confirmation
                clear();
                mvprintw(5, 5, "Task Manager Killer ENABLED!");
                mvprintw(6, 5, "Press any key to continue...");
                refresh();
                getch();
            }
        }},
        {"Status", []() {
            clear();
            mvprintw(5, 5, "Task Manager Killer Status:");
            mvprintw(6, 5, killerActive.load() ? "ENABLED (Running)" : "DISABLED (Stopped)");
            mvprintw(8, 5, "Press any key to continue...");
            refresh();
            getch();
        }}
    };
    
    menu.executeMenu(taskKillerFunctions);
}
