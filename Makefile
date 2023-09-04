EXE := main
SRC_DIR := src
BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug
RELEASE_DIR := $(BUILD_DIR)/release

CXXFLAGS := -std=c++17 -Iinclude -MMD -Wall
CXXFLAGS_DEBUG := -g
CXXFLAGS_RELEASE := -DNDEBUG -O3

.PHONY: all release clean

all: $(DEBUG_DIR)/$(EXE)
release: $(RELEASE_DIR)/$(EXE)

$(BUILD_DIR):
	@printf "%s\n" $(CXXFLAGS) > compile_flags.txt
	mkdir -p $@

# DEBUG TARGETS
$(DEBUG_DIR): | $(BUILD_DIR)
	mkdir -p $@

OBJ_DEBUG := $(patsubst $(SRC_DIR)/%.cpp,$(DEBUG_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp))

$(DEBUG_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_DIR)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_DEBUG) -c $< -o $@

$(DEBUG_DIR)/$(EXE): $(OBJ_DEBUG)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

-include $(OBJ_DEBUG:.o=.d)

# RELEASE TARGETS
$(RELEASE_DIR): | $(BUILD_DIR)
	mkdir -p $@

OBJ_RELEASE := $(patsubst $(SRC_DIR)/%.cpp,$(RELEASE_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp))

$(RELEASE_DIR)/%.o: $(SRC_DIR)/%.cpp | $(RELEASE_DIR)
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_RELEASE) -c $< -o $@

$(RELEASE_DIR)/$(EXE): $(OBJ_RELEASE)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) compile_flags.txt

-include $(OBJ_RELEASE:.o=.d)
