CC=gcc
CFLAGS=-Wall
CLIBS=-lsqlite3
OBJECTS=main.o

all: build
build: $(OBJECTS)
	$(CC) $(CFLAGS) main.c $(CLIBS) -o main.o


