CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra

BUILD_DIR = build
COMMON_SRC = src/network_utils.c src/crypto_utils.c
GTK_FLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)

.PHONY: all host arm64 run-server run-client clean

all: host

host: $(BUILD_DIR)/server $(BUILD_DIR)/client_gui

arm64: $(BUILD_DIR)/server-arm64 $(BUILD_DIR)/client_gui-arm64

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/server: src/server.c $(COMMON_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iinclude $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/client_gui: src/client_gui.c $(COMMON_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iinclude $^ -o $@ $(GTK_FLAGS) $(LDFLAGS)

$(BUILD_DIR)/server-arm64: src/server.c $(COMMON_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iinclude $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/client_gui-arm64: src/client_gui.c $(COMMON_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Iinclude $^ -o $@ $(GTK_FLAGS) $(LDFLAGS)

run-server: $(BUILD_DIR)/server
	./$(BUILD_DIR)/server

run-client: $(BUILD_DIR)/client_gui
	./$(BUILD_DIR)/client_gui

clean:
	$(RM) $(BUILD_DIR)/server $(BUILD_DIR)/client_gui \
	      $(BUILD_DIR)/server-arm64 $(BUILD_DIR)/client_gui-arm64
