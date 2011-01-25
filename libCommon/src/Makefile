INCLUDE_FLAGS=-I../include -I/usr/src/linux-headers-2.6.32-27/include
INCLUDE_DIRS=$(subst -I,,$(INCLUDE_FLAGS))
COMPILE_FLAGS=-c -Wall -Wextra -Werror -fno-default-inline -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS=-rcv
LIBRARY_DIR=../bin
LIBRARY_FILE=libCommon.a
OBJECT_FILES=Exception.o Log.o LinuxTimer.o LinuxEvent.o LinuxFunctions.o LinuxSemaphore.o LinuxMutex.o LinuxPipe.o LinuxThread.o LinuxHandleSet.o
HEADER_FILES=$(shell find $(INCLUDE_DIRS) -name "*.h")

all: $(LIBRARY_DIR)/$(LIBRARY_FILE)

clean:
	rm -rf $(LIBRARY_DIR)/$(LIBRARY_FILE) $(OBJECT_FILES)

$(LIBRARY_DIR)/$(LIBRARY_FILE): $(OBJECT_FILES)
	mkdir -p $(LIBRARY_DIR)
	ar $(LINKER_FLAGS) $@ $(OBJECT_FILES)

# TODO: try to get this working for EFD_WAITREAD
#eventfd.o: eventfd.c ../include/eventfd-extension.h
#	g++ $(COMPILE_FLAGS) eventfd.c -o eventfd.o

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

