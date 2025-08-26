CXX = g++
CXXFLAGS = -Wall -O2
LDFLAGS = -lncursesw

SRC = main.cpp \
      modules/htop.cpp \
      modules/taskkiller.cpp \
      modules/screenfetch.cpp \
      modules/lsblk.cpp \
      modules/settings.cpp \
      modules/sudo_settings.cpp \
      modules/input_devices.cpp \
      modules/uname_info.cpp \
      modules/network_monitor.cpp \
      menu.cpp \

OBJ = $(SRC:.cpp=.o)

toolbox: $(OBJ)
	$(CXX) $(CXXFLAGS) -o terminstant $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) terminstant
