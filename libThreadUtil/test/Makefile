INCLUDE_FLAGS=-I../include -I../../thirdParty/catch
INCLUDE_DIRS=$(subst -I,,$(INCLUDE_FLAGS))
HEADER_FILES=$(shell find $(INCLUDE_DIRS) -name "*.h")
COMPILE_FLAGS=-c -Wall -Wextra -Werror -fno-default-inline -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS=-L/usr/lib/gcc/i486-linux-gnu/4.3.2/ -lrt -lstdc++
BINARY_DIR=../bin
BINARY_FILE=libThreadUtilTest

OBJECT_FILES=testMain.o

INCLUDE_LIBS=../bin/libThreadUtil.a \
             ../../libCommon/bin/libCommon.a \
             ../../libThreadComm/bin/libThreadComm.a \
             ../../libProcessComm/bin/libProcessComm.a \
             ../../libSocketComm/bin/libSocketComm.a

all: $(BINARY_DIR)/$(BINARY_FILE)

clean:
	rm -rf $(BINARY_DIR)/$(BINARY_FILE) $(OBJECT_FILES)

runTest: all
	$(BINARY_DIR)/$(BINARY_FILE)

valTest: all
	valgrind --track-origins=yes --track-fds=yes $(BINARY_DIR)/$(BINARY_FILE)

$(BINARY_DIR)/$(BINARY_FILE): $(OBJECT_FILES) $(INCLUDE_LIBS)
	mkdir -p $(BINARY_DIR)
	gcc $(LINKER_FLAGS) -o $@ $(OBJECT_FILES) $(INCLUDE_LIBS)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

