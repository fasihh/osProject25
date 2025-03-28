CC = gcc
CFLAGS = -Wall -Wextra
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = obj
TARGET_SERVER = server
TARGET_CLIENT = client
FILE_TYPE = c

SRC_FILES := $(wildcard $(SRC_DIR)/*.$(FILE_TYPE))
OBJ_FILES := $(patsubst $(SRC_DIR)/%.$(FILE_TYPE), $(OBJ_DIR)/%.o, $(SRC_FILES))

SERVER_MAIN = $(SRC_DIR)/$(TARGET_SERVER).$(FILE_TYPE)
CLIENT_MAIN = $(SRC_DIR)/$(TARGET_CLIENT).$(FILE_TYPE)

SERVER_OBJ_FILES := $(filter-out $(OBJ_DIR)/client.o, $(OBJ_FILES))
CLIENT_OBJ_FILES := $(filter-out $(OBJ_DIR)/server.o, $(OBJ_FILES))

$(BIN_DIR)/$(TARGET_SERVER): $(SERVER_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

$(BIN_DIR)/$(TARGET_CLIENT): $(CLIENT_OBJ_FILES)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(FILE_TYPE)
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