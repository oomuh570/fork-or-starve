CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = dining_philosophers
SRCS = src/main.c

all: $(TARGET)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
clean:
	rm -f $(TARGET) src/*.o
.PHONY: all clean