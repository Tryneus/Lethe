INCLUDE_FLAGS=-I../include -I../../libCommon/include
INCLUDE_DIRS=$(subst -I,,$(INCLUDE_FLAGS))
COMPILE_FLAGS=-c -Wall -Wextra -Werror -O3 -g $(INCLUDE_FLAGS)
LINKER_FLAGS=-rcv
LIBRARY_DIR=../bin
LIBRARY_FILE=libThreadComm.a
OBJECT_FILES=Channel.o Connection.o Header.o List.o Message.o ReceiveList.o UnallocList.o
HEADER_FILES=$(shell find $(INCLUDE_DIRS) -name "*.h")

all: $(LIBRARY_DIR)/$(LIBRARY_FILE)

clean:
	rm -rf $(LIBRARY_DIR)/$(LIBRARY_FILE) $(OBJECT_FILES)

$(LIBRARY_DIR)/$(LIBRARY_FILE): $(OBJECT_FILES)
	mkdir -p $(LIBRARY_DIR)
	ar $(LINKER_FLAGS) $@ $(OBJECT_FILES)

%.o: %.cpp $(HEADER_FILES)
	g++ $(COMPILE_FLAGS) $< -o $@

