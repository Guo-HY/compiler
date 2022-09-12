SRC_DIR = src
OBJ_DIR = obj

DIRS = $(shell find $(SRC_DIR) -type d)
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
HEADERS = $(shell find $(SRC_DIR) -name '*.hpp')
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJ_DIRS = $(subst src,obj,$(DIRS) )

CXX = g++
CXXFLAGS = -std=c++17  -Wall

.PHONY: clean

gsc: obj $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) -o $@

obj:
	mkdir -p $(OBJ_DIRS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(addprefix -I,${DIRS}) -c $< -o $@

clean:
	rm $(OBJS)