INCLUDE_FLAGS=-I../include -I../../libCommon/include
INCLUDE_DIRS=$(subst -I,,$(INCLUDE_FLAGS))
COMPILE_FLAGS=-c -Wall -Wextra -Werror -fno-default-inline -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS=-L/usr/lib/gcc/i486-linux-gnu/4.3.2/ -lrt -lstdc++
BINARY_DIR=../bin
BINARY_FILE=threadCommTest
OBJECT_FILES=EchoThread.o SenderThread.o ThreadCommTest.o
INCLUDE_LIBS=../../libCommon/bin/libCommon.a ../bin/libThreadComm.a
HEADER_FILES=$(shell find $(INCLUDE_DIRS) -name "*.h")

all: $(BINARY_DIR)/$(BINARY_FILE)

clean:
	rm -rf $(BINARY_DIR)/$(BINARY_FILE) $(OBJECT_FILES)

$(BINARY_DIR)/$(BINARY_FILE): $(OBJECT_FILES) $(INCLUDE_LIBS)
	mkdir -p $(BINARY_DIR)
	gcc $(LINKER_FLAGS) -o $@ $(OBJECT_FILES) $(INCLUDE_LIBS)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

