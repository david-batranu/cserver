CC=gcc
CFLAGS=-Wall -ggdb -std=c89 -pedantic -x c
CLIBS=-lsqlite3
OBJECTS=main.o

all: build
build: $(OBJECTS)
	$(CC) $(CFLAGS) routes.c queries.c main.c $(CLIBS) -o main.o


