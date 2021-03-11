CC=gcc
CFLAGS=-Wall -g -pedantic

SRC = $(wildcard ./src/*.c)

TEST_SRC = $(wildcard ./test/*.c)

TARGET_NAME = ./out/target_build

LIBS=-I./lib/spookyhash -L./lib/spookyhash -lspookyhash

build: $(SRC)
	$(CC) $(CFLAGS) -c $^ $(LIBS) -o ./out/hashmap.o

test: build
	$(CC) $(CFLAGS) $(TEST_SRC) ./out/hashmap.o $(LIBS) -o ./out/test
	./out/test
