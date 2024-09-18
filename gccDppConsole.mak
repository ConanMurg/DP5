
# Makefile - gccDppConsoleMiniX2

# Default to no (-O0) optimisation for Debug, Release has -O2 optimisation
ifndef CFG
CFG = Debug
endif

CC = gcc
CXX = g++
CFLAGS =
CXXFLAGS = $(CFLAGS)

# Compiler flags
ifeq ($(CFG), Debug)
	CFLAGS += -W -I./ -I/usr/include/libusb-1.0 -O0 -fexceptions -I./DeviceIO/ -g -fno-inline -D_DEBUG -D_CONSOLE
else
	CFLAGS += -W -I./ -I/usr/include/libusb-1.0 -O2 -fexceptions -I./DeviceIO/ -g -fno-inline -DNDEBUG -D_CONSOLE
endif

# Linker flags
LDFLAGS = #-shared # Flag for creating shared object (.so)

LDLIBS = -lusb-1.0
# LDLIBS = -L/usr/include/libusb-1.0 -lusb-1.0
LIBS = -L/usr/lib/x86_64-linux-gnu -lusb-1.0

# Target name for shared object
TARGET_SO = libgccDppConsoleDP5.so
TARGET_SO = libgccDppConsoleMiniX2.so

# Source files (add all your source files here)
SOURCE_FILES = \
	./ConsoleHelper.cpp \
	./DeviceIO/AsciiCmdUtilities.cpp \
	./DeviceIO/DP5Protocol.cpp \
	./DeviceIO/DP5Status.cpp \
	./DeviceIO/DppUtilities.cpp \
	./DeviceIO/ParsePacket.cpp \
	./DeviceIO/SendCommand.cpp \
	./DeviceIO/DppLibUsb.cpp \
	./stringex.cpp \
	./gccDppConsoleMX2.cpp

# Header files (add all your header files here)
HEADER_FILES = \
	./ConsoleHelper.h \
	./DeviceIO/AsciiCmdUtilities.h \
	./DeviceIO/DP5Protocol.h \
	./DeviceIO/DP5Status.h \
	./DeviceIO/DppConst.h \
	./DeviceIO/DppUtilities.h \
	./DeviceIO/ParsePacket.h \
	./DeviceIO/SendCommand.h \
	./DeviceIO/DppLibUsb.h \
	./DeviceIO/libusb.h \
	./stringex.h \
	./stringSplit.h

# Object files (automatically generated from source files)
OBJ_FILES = $(patsubst %.cpp,%.o,$(SOURCE_FILES))

# Dependency file
DEP_FILE = gccDppConsole.dep

# Rules
all: $(TARGET_SO)

# Rule for compiling source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $< -o $@

# Rule for linking object files into shared object (.so)
$(TARGET_SO): $(OBJ_FILES)
	$(CXX) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

# Cleanup rule
clean:
	rm -f $(OBJ_FILES) $(TARGET_SO) $(DEP_FILE)

# Dependency generation rule
depends:
	$(CXX) $(CXXFLAGS) -MM $(SOURCE_FILES) > $(DEP_FILE)

# Include dependency file if it exists
-include $(DEP_FILE)
