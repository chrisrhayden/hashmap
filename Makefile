CC=gcc
CFLAGS=-Wall -g -pedantic

SRC = $(wildcard ./src/*.c)

TEST_SRC = $(wildcard ./test/*.c)

OBJ_NAME = ./out/hashmap.o

STD_LIBS = -lm

build: $(SRC)
	$(CC) $(CFLAGS) -c $^ $(STD_LIBS) -o $(OBJ_NAME)

test: build
	$(CC) $(CFLAGS) $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test

run_test: test
	./out/test
