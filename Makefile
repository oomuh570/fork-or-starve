CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = dining_philosophers
SRCS =  src/main.c \
	src/philosopher.c \
	src/forks.c \
	src/semaphore.c \
	src/display.c \
	src/stats.c

all: $(TARGET)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
clean:
	rm -f $(TARGET) src/*.o
.PHONY: all clean
