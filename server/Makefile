CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -O2 -I../common/
TARGET = server
SRC = server.c client_list.c main.c ../common/protocol.c
LIBS = -lpthread

all: run

build:
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) $(LIBS)

run: build
	./$(TARGET)

clean:
	rm -f $(TARGET)
