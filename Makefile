CC=gcc
CFLAGS=-Wall -ggdb -std=c89 -pedantic -x c
CLIBS=-lpthread
OBJECTS=main.o

all: build
build: $(OBJECTS)
	$(CC) $(CFLAGS) sqlite/sqlite3.c utils.c request.c query_handlers.c route_handlers.c routes.c queries.c main.c $(CLIBS) -o main.o


