INCLUDE_FLAGS:=-I../include \
               -I../../thirdParty/catch \
               -I../../libCommon/include

INCLUDE_DIRS :=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES :=$(shell find $(INCLUDE_DIRS) -name '*.h*')

COMPILE_FLAGS:=-c -Wall -Wextra -Wno-sign-compare -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS :=-L/usr/lib/gcc/i486-linux-gnu/4.3.2/ -lrt -lstdc++

BINARY_DIR   :=../bin
BINARY_FILE  :=$(BINARY_DIR)/libThreadCommTest

OBJECT_FILES :=testMain.o \
               EchoThread.o \
               SenderThread.o

INCLUDE_LIBS :=../bin/libThreadComm.a \
               ../../libCommon/bin/libCommon.a

all: $(BINARY_FILE)

clean:
	rm -rf $(BINARY_FILE) $(OBJECT_FILES) runTest.log valTest.log

runTest: all
	$(BINARY_FILE) 2>&1 | tee runTest.log

valTest: all
	valgrind --track-origins=yes --track-fds=yes $(BINARY_FILE) 2>&1 | tee valTest.log

$(BINARY_FILE): $(OBJECT_FILES) $(INCLUDE_LIBS)
	mkdir -p $(BINARY_DIR)
	gcc $(LINKER_FLAGS) -o $@ $(OBJECT_FILES) $(INCLUDE_LIBS)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

