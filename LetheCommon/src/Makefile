INCLUDE_FLAGS:=-I../include \
               -I../../thirdParty

INCLUDE_DIRS :=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES :=$(shell find $(INCLUDE_DIRS) -name '*.h*' 2>&-)

COMPILE_FLAGS:=-c -std=c++0x -Wall -Wextra -Werror -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS :=-rcv

LIBRARY_DIR  :=../bin
LIBRARY_FILE :=$(LIBRARY_DIR)/LetheCommon.a

OBJECT_FILES :=LetheException.o \
               LetheInternal.o \
               BaseThread.o \
               WaitObject.o \
               ByteStream.o \
               MessageStream.o \
               Log.o \
               linux/LinuxAtomic.o \
               linux/LinuxTimer.o \
               linux/LinuxEvent.o \
               linux/LinuxFunctions.o \
               linux/LinuxSemaphore.o \
               linux/LinuxMutex.o \
               linux/LinuxPipe.o \
               linux/LinuxThread.o \
               linux/LinuxWaitSet.o \
               linux/LinuxSharedMemory.o

all: $(LIBRARY_FILE)

clean:
	rm -rf $(LIBRARY_FILE) $(OBJECT_FILES)

$(LIBRARY_FILE): $(OBJECT_FILES)
	mkdir -p $(LIBRARY_DIR)
	ar $(LINKER_FLAGS) $@ $(OBJECT_FILES)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

