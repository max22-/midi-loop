EXEC = midi-loop
BIN_DIR = $(PWD)/bin
BUILD_DIR = $(PWD)/build
SRC_DIRS = src

SRCS = $(shell find $(SRC_DIRS) -name *.cpp)
OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

INC_DIRS = include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS = $(INC_FLAGS) -Wall -MMD -MP
LDFLAGS = -lsfml-audio -lsfml-system -lportmidi

CPP = g++

all: $(BIN_DIR)/$(EXEC)

$(BIN_DIR)/$(EXEC): $(OBJS)
	mkdir -p bin
	$(CPP) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

.PHONY: clean run

run: $(BIN_DIR)/$(EXEC)
	$(BIN_DIR)/$(EXEC)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)/$(EXEC)

-include $(DEPS)
