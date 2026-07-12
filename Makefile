CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2
TARGET = server
SRC = server.c client_list.c main.c protocol.c
LIBS = -lpthread

all: run

build:
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) $(LIBS)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
