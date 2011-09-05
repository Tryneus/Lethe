INCLUDE_FLAGS:=-I../include \
               -I../../thirdParty

INCLUDE_DIRS :=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES :=$(shell find $(INCLUDE_DIRS) -name '*.h*' 2>&-)

COMPILE_FLAGS:=-c -std=c++0x -Wall -Wextra -Wno-sign-compare -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS :=-L/usr/lib/gcc/i486-linux-gnu/4.3.2/ -lrt -lstdc++

BINARY_DIR   :=../bin
BINARY_FILE  :=$(BINARY_DIR)/LetheCommonTest

OBJECT_FILES :=testMain.o \
               testFunctions.o \
               testWaitSet.o \
               testPipe.o \
               testTimer.o \
               testEvent.o \
               testMutex.o \
               testThread.o \
               testSemaphore.o \
               testLog.o \
               testSharedMemory.o

INCLUDE_LIBS :=../bin/LetheCommon.a

all: $(BINARY_FILE)

clean:
	rm -rf $(BINARY_FILE) $(OBJECT_FILES) runTest.log valTest.log

runTest: all
	$(BINARY_FILE) 2>&1 | tee runTest.log

valTest: all
	valgrind --leak-check=full --sim-hints=lax-ioctls --show-reachable=yes --track-origins=yes --track-fds=yes $(BINARY_FILE) 2>&1 | tee valTest.log

$(BINARY_FILE): $(OBJECT_FILES) $(INCLUDE_LIBS)
	mkdir -p $(BINARY_DIR)
	gcc $(LINKER_FLAGS) -o $@ $(OBJECT_FILES) $(INCLUDE_LIBS)

testMain.o: testMain.cpp
	g++ $(COMPILE_FLAGS) $< -o $@

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

