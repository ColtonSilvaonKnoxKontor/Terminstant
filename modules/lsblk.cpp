// 2025 Colton Silva
//
// This is part of Terminstant, a menu-driven tool for launching text-based program
// without entering command line
//
// lsblk does not output special characters inside ncurses session, so we
// have to convert it as ASCII alternatives.
#include <ncursesw/ncurses.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <wchar.h>
#include <map>
#include <functional>
#include "../menu.hpp"

using namespace std;

static void renderLsblkOutput() {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);

	// Use lsblk with tree view and proper encoding
	FILE* pipe = popen("LC_ALL=C lsblk -o NAME,MAJ:MIN,RM,SIZE,RO,TYPE,MOUNTPOINTS", "r");
	if (pipe) {
		char buffer[1024];
		int line = 0;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			text.erase(remove(text.begin(), text.end(), '\r'), text.end());
			// Trim trailing newline
			if (!text.empty() && text.back() == '\n') text.pop_back();
			
			// Convert UTF-8 box-drawing characters to ASCII alternatives
			// The UTF-8 sequences are: ├─ (0xE2 0x94 0x9C 0xE2 0x94 0x80), └─ (0xE2 0x94 0x94 0xE2 0x94 0x80), │ (0xE2 0x94 0x82)
			size_t pos = 0;
			while ((pos = text.find("\xE2\x94\x9C\xE2\x94\x80")) != std::string::npos) {
				text.replace(pos, 6, "|--"); // Replace ├─ with |--
			}
			while ((pos = text.find("\xE2\x94\x94\xE2\x94\x80")) != std::string::npos) {
				text.replace(pos, 6, "`--"); // Replace └─ with `--
			}
			while ((pos = text.find("\xE2\x94\x82")) != std::string::npos) {
				text.replace(pos, 3, "|");   // Replace │ with |
			}
			
			// Truncate to fit screen width
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line, 0, "%s", text.c_str());
			line++;
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

static void renderScsiDevicesOutput() {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);

	// Use lsblk -S to show SCSI devices
	FILE* pipe = popen("LC_ALL=C lsblk -S -o NAME,MAJ:MIN,RM,SIZE,RO,TYPE,MODEL,VENDOR", "r");
	if (pipe) {
		char buffer[1024];
		int line = 0;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			text.erase(remove(text.begin(), text.end(), '\r'), text.end());
			// Trim trailing newline
			if (!text.empty() && text.back() == '\n') text.pop_back();
			
			// Truncate to fit screen width
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line, 0, "%s", text.c_str());
			line++;
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

static void renderNvmeDevicesOutput() {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);

	// Use lsblk to show NVMe devices (filter by name starting with nvme)
	FILE* pipe = popen("LC_ALL=C lsblk -o NAME,MAJ:MIN,RM,SIZE,RO,TYPE,MODEL,VENDOR | grep -E '^nvme'", "r");
	if (pipe) {
		char buffer[1024];
		int line = 0;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			text.erase(remove(text.begin(), text.end(), '\r'), text.end());
			// Trim trailing newline
			if (!text.empty() && text.back() == '\n') text.pop_back();
			
			// Truncate to fit screen width
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line, 0, "%s", text.c_str());
			line++;
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

static void renderVirtioDevicesOutput() {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);

	// Use lsblk to show virtio devices (filter by name starting with vd)
	FILE* pipe = popen("LC_ALL=C lsblk -o NAME,MAJ:MIN,RM,SIZE,RO,TYPE,MODEL,VENDOR | grep -E '^vd'", "r");
	if (pipe) {
		char buffer[1024];
		int line = 0;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			text.erase(remove(text.begin(), text.end(), '\r'), text.end());
			// Trim trailing newline
			if (!text.empty() && text.back() == '\n') text.pop_back();
			
			// Truncate to fit screen width
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line, 0, "%s", text.c_str());
			line++;
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

static void renderTopologyInfoOutput() {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);

	// Use lsblk -t to show topology information with tree structure
	FILE* pipe = popen("LC_ALL=C lsblk -t -o NAME,MAJ:MIN,RM,SIZE,RO,TYPE,MOUNTPOINT,ALIGNMENT,MIN-IO,OPT-IO,PHY-SEC,LOG-SEC,ROTA,SCHED,RQ-SIZE,DISC-ALN,DISC-GRAN,DISC-MAX,DISC-ZERO", "r");
	if (pipe) {
		char buffer[1024];
		int line = 0;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			text.erase(remove(text.begin(), text.end(), '\r'), text.end());
			// Trim trailing newline
			if (!text.empty() && text.back() == '\n') text.pop_back();
			
			// Convert UTF-8 box-drawing characters to ASCII alternatives for topology tree
			size_t pos = 0;
			while ((pos = text.find("\xE2\x94\x9C\xE2\x94\x80")) != std::string::npos) {
				text.replace(pos, 6, "|--"); // Replace ├─ with |--
			}
			while ((pos = text.find("\xE2\x94\x94\xE2\x94\x80")) != std::string::npos) {
				text.replace(pos, 6, "`--"); // Replace └─ with `--
			}
			while ((pos = text.find("\xE2\x94\x82")) != std::string::npos) {
				text.replace(pos, 3, "|");   // Replace │ with |
			}
			
			// Truncate to fit screen width
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line, 0, "%s", text.c_str());
			line++;
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

static void renderFilesystemInfoOutput() {
	clear();
	refresh();
	int maxLines, maxCols;
	getmaxyx(stdscr, maxLines, maxCols);

	// Use lsblk -f to show filesystem information
	FILE* pipe = popen("LC_ALL=C lsblk -f -o NAME,FSTYPE,FSVER,MOUNTPOINT,LABEL,UUID", "r");
	if (pipe) {
		char buffer[1024];
		int line = 0;
		while (fgets(buffer, sizeof(buffer), pipe) && line < maxLines - 2) {
			std::string text = buffer;
			text.erase(remove(text.begin(), text.end(), '\r'), text.end());
			// Trim trailing newline
			if (!text.empty() && text.back() == '\n') text.pop_back();
			
			// Convert UTF-8 box-drawing characters to ASCII alternatives for filesystem tree
			size_t pos = 0;
			while ((pos = text.find("\xE2\x94\x9C\xE2\x94\x80")) != std::string::npos) {
				text.replace(pos, 6, "|--"); // Replace ├─ with |--
			}
			while ((pos = text.find("\xE2\x94\x94\xE2\x94\x80")) != std::string::npos) {
				text.replace(pos, 6, "`--"); // Replace └─ with `--
			}
			while ((pos = text.find("\xE2\x94\x82")) != std::string::npos) {
				text.replace(pos, 3, "|");   // Replace │ with |
			}
			
			// Truncate to fit screen width
			if ((int)text.size() > maxCols - 1) text = text.substr(0, (size_t)(maxCols - 1));
			mvprintw(line, 0, "%s", text.c_str());
			line++;
		}
		pclose(pipe);
	}
	mvprintw(maxLines - 1, 0, "Press any key to continue...");
	refresh();
	getch();
}

void showLsblkMenu() {
	vector<string> options = {
		"Show Block Devices",
		"Show SCSI Devices",
		"Show NVMe Devices",
		"Show virtio devices",
		"Show Topology Info",
		"Show Filesystem Info",
		"Back"
	};
	Menu menu("=== Block Devices ===", options);
	
	// Function mapping for lsblk menu
	std::map<std::string, std::function<void()>> lsblkFunctions = {
		{"Show Block Devices", renderLsblkOutput},
		{"Show Filesystem Info", renderFilesystemInfoOutput},
		{"Show NVMe Devices", renderNvmeDevicesOutput},
		{"Show SCSI Devices", renderScsiDevicesOutput},
		{"Show Topology Info", renderTopologyInfoOutput},
		{"Show virtio devices", renderVirtioDevicesOutput}
	};
	
	menu.executeMenu(lsblkFunctions);
}
