INCLUDE_FLAGS:=-I../include \
               -I../../thirdParty/mct/include

INCLUDE_DIRS :=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES :=$(shell find $(INCLUDE_DIRS) -name '*.h*')

COMPILE_FLAGS:=-c -Wall -Wextra -Werror -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS :=-rcv

LIBRARY_DIR  :=../bin
LIBRARY_FILE :=$(LIBRARY_DIR)/libCommon.a

OBJECT_FILES :=Log.o \
               AbstractionException.o \
               LinuxTimer.o \
               LinuxEvent.o \
               LinuxFunctions.o \
               LinuxSemaphore.o \
               LinuxMutex.o \
               LinuxPipe.o \
               LinuxThread.o \
               LinuxWaitSet.o \
               BaseThread.o \
               WaitObject.o

all: $(LIBRARY_FILE)

clean:
	rm -rf $(LIBRARY_FILE) $(OBJECT_FILES)

$(LIBRARY_FILE): $(OBJECT_FILES)
	mkdir -p $(LIBRARY_DIR)
	ar $(LINKER_FLAGS) $@ $(OBJECT_FILES)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

