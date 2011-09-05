INCLUDE_FLAGS:=-I../include \
               -I../../thirdParty \
               -I../../LetheCommon/include \
               -I../../LetheThreadComm/include

INCLUDE_DIRS :=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES :=$(shell find $(INCLUDE_DIRS) -name '*.h*' 2>&-)

COMPILE_FLAGS:=-c -std=c++0x -Wall -Wextra -Wno-sign-compare -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS :=-L/usr/lib/gcc/i486-linux-gnu/4.3.2/ -lrt -lstdc++

BINARY_DIR   :=../bin
BINARY_FILE  :=$(BINARY_DIR)/LetheProcessCommTest

OBJECT_FILES :=testMain.o \
               testByteStream.o \
               testMessageStream.o

INCLUDE_LIBS :=../bin/LetheProcessComm.a \
               ../../LetheThreadComm/bin/LetheThreadComm.a \
               ../../LetheCommon/bin/LetheCommon.a

all: $(BINARY_FILE) testProcess

clean:
	rm -rf $(BINARY_FILE) $(OBJECT_FILES) check.log valCheck.log

check: all
	$(BINARY_FILE) 2>&1 | tee check.log

valCheck: all
	valgrind --track-origins=yes --track-fds=yes $(BINARY_FILE) 2>&1 | tee valCheck.log

# Another rule to make the test process a different binary
testProcess: testProcess.o $(INCLUDE_LIBS)
	mkdir -p $(BINARY_DIR)
	gcc $(LINKER_FLAGS) -o ../bin/testProcess testProcess.o $(INCLUDE_LIBS)

$(BINARY_FILE): $(OBJECT_FILES) $(INCLUDE_LIBS)
	mkdir -p $(BINARY_DIR)
	gcc $(LINKER_FLAGS) -o $@ $(OBJECT_FILES) $(INCLUDE_LIBS)

testMain.o: testMain.cpp
	g++ $(COMPILE_FLAGS) $< -o $@

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

