CXX = g++
CXXFLAGS = -Wall -O2
LDFLAGS = -lncursesw

SRC = main.cpp \
      modules/htop.cpp \
      modules/taskkiller.cpp \
      modules/screenfetch.cpp \
      modules/lsblk.cpp \
      modules/ls.cpp \
      modules/settings.cpp \
      modules/sudo_settings.cpp \
      modules/input_devices.cpp \
      modules/uname_info.cpp \
      modules/network_monitor.cpp \
      modules/apt_package_management.cpp \
      menu.cpp \

OBJ = $(SRC:.cpp=.o)

toolbox: $(OBJ)
	$(CXX) $(CXXFLAGS) -o terminstant $(OBJ) $(LDFLAGS)

check: toolbox
	@echo "Running terminstant integration tests..."
	@./term_integration_test.sh

check-unit: toolbox
	@echo "Running file management unit tests..."
	@./test_file_management.sh

help:
	@echo "Available targets:"
	@echo "  toolbox    - Build the terminstant executable"
	@echo "  check      - Run terminstant integration tests"
	@echo "  check-unit - Run file management unit tests"
	@echo "  clean      - Remove all build files"
	@echo "  help       - Show this help message"

clean:
	rm -f $(OBJ) terminstant

.PHONY: toolbox check check-unit clean help
