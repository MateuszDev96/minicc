CFLAGS = -std=c11 -g -fno-common
BUILD_DIR = ../build
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

# Final binary in ../build/minicc
$(BUILD_DIR)/minicc: $(OBJS)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .c file to ../build/*.o
$(BUILD_DIR)/%.o: %.c minicc.h
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Alias target so you can still run: make minicc
minicc: $(BUILD_DIR)/minicc

# Optional test target
test: minicc
	./test.sh

clean:
	rm -rf $(BUILD_DIR) *~ tmp*

.PHONY: test clean
