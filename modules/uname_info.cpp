// 2025 Colton Silva
// Terminstant - uname info submenu

#include <ncurses.h>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cstdio>
#include <algorithm>
#include "../menu.hpp"
#include "uname_info.hpp"

static void runAndDisplay(const char* title, const char* cmd) {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);
	mvprintw(0, 0, "%s", title);
	FILE* pipe = popen(cmd, "r");
	if (pipe) {
		char buffer[512];
		int line = 2;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			if (!text.empty() && text.back() == '\n') text.pop_back();
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line++, 0, "%s", text.c_str());
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

void showUnameInfoMenu() {
	std::vector<std::string> options = {
		"All",
		"Hardware Platform",
		"Hostname",
		"Kernel Name",
		"Kernel Release",
		"Kernel Version",
		"Machine Hardware Name",
		"Operating System",
		"Processor Type",
		"Back"
	};
	Menu menu("=== Information via uname ===", options);

	// Function map; Menu auto-sorts, mapping uses option string keys
	std::map<std::string, std::function<void()>> fn = {
		{"All", [](){ runAndDisplay("=== uname -a ===", "uname -a"); }},
		{"Hardware Platform", [](){ runAndDisplay("=== uname -i ===", "uname -i"); }},
		{"Hostname", [](){ runAndDisplay("=== uname -n ===", "uname -n"); }},
		{"Kernel Name", [](){ runAndDisplay("=== uname -s ===", "uname -s"); }},
		{"Kernel Release", [](){ runAndDisplay("=== uname -r ===", "uname -r"); }},
		{"Kernel Version", [](){ runAndDisplay("=== uname -v ===", "uname -v"); }},
		{"Machine Hardware Name", [](){ runAndDisplay("=== uname -m ===", "uname -m"); }},
		{"Operating System", [](){ runAndDisplay("=== uname -o ===", "uname -o"); }},
		{"Processor Type", [](){ runAndDisplay("=== uname -p ===", "uname -p"); }}
	};

	menu.executeMenu(fn);
}
