CC = g++
CFLAGS = -Wall -Wextra -std=c++17
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = obj
TARGET_SERVER = server
TARGET_CLIENT = client

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRC_FILES))

SERVER_MAIN = $(SRC_DIR)/server_main.cpp
CLIENT_MAIN = $(SRC_DIR)/client_main.cpp

SERVER_OBJ_FILES := $(filter-out $(OBJ_DIR)/client_main.o, $(OBJ_FILES))
CLIENT_OBJ_FILES := $(filter-out $(OBJ_DIR)/server_main.o, $(OBJ_FILES))

$(BIN_DIR)/$(TARGET_SERVER): $(SERVER_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

$(BIN_DIR)/$(TARGET_CLIENT): $(CLIENT_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

.PHONY: all clean rs rc

all: $(BIN_DIR)/$(TARGET_SERVER) $(BIN_DIR)/$(TARGET_CLIENT)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

rs: $(BIN_DIR)/$(TARGET_SERVER)
	./$(BIN_DIR)/$(TARGET_SERVER)

rc: $(BIN_DIR)/$(TARGET_CLIENT)
	./$(BIN_DIR)/$(TARGET_CLIENT)
