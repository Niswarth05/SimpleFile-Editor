CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET = editor
SOURCES = main.c editor.c

all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)