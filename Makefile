CXX      ?= em++
CC       ?= emcc
CXXFLAGS ?= -O2 -std=c++17 -DNBUILD -DNDEBUG -DQUIET
CFLAGS   ?= -O2 -DNBUILD -DNDEBUG -DQUIET

CADICAL_CPP := $(filter-out cadical/src/cadical.cpp cadical/src/mobical.cpp, \
                 $(wildcard cadical/src/*.cpp))
CADICAL_C   := $(wildcard cadical/src/*.c)

CPP_OBJS := $(patsubst %.cpp, build/%.o, $(notdir $(CADICAL_CPP) wrapper.cpp))
C_OBJS   := $(patsubst %.c, build/%.o, $(notdir $(CADICAL_C)))
OBJS     := $(CPP_OBJS) $(C_OBJS)

BUILD_DIR := build

EMFLAGS := -s EXPORTED_FUNCTIONS='["_solve_dimacs","_free_result","_malloc","_free"]' \
           -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","allocateUTF8"]' \
           -s MODULARIZE=1 \
           -s EXPORT_NAME='createCadical' \
           -s ALLOW_MEMORY_GROWTH=1 \
           -s ENVIRONMENT='web,worker,node'

.PHONY: all clean

all: $(BUILD_DIR)/cadical.js

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile CaDiCaL C++ source files
$(BUILD_DIR)/%.o: cadical/src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I cadical/src -c $< -o $@

# Compile CaDiCaL C source files
$(BUILD_DIR)/%.o: cadical/src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I cadical/src -c $< -o $@

# Compile wrapper
$(BUILD_DIR)/wrapper.o: wrapper.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I . -c $< -o $@

# Link everything into JS + WASM
$(BUILD_DIR)/cadical.js: $(OBJS)
	$(CXX) $(CXXFLAGS) $(EMFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)
