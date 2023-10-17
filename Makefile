CC=gcc
CFLAGS=-Wall -ggdb -std=c89 -pedantic
CLIBS=-lsqlite3
OBJECTS=main.o

all: build
build: $(OBJECTS)
	$(CC) $(CFLAGS) main.c $(CLIBS) -o main.o


