CC = gcc
CFLAGS = -Wall -g -pedantic

OPTIMIZATION = -o2

SRC = $(wildcard ./src/*.c)

TEST_SRC = $(wildcard ./test/*.c)

OBJ_NAME = ./out/hashmap.o

STD_LIBS = -lm

build: $(SRC)
	$(CC) $(CFLAGS) -c $^ $(STD_LIBS) -o $(OBJ_NAME)

build_ox: $(SRC)
	$(CC) $(CFLAGS) $(OPTIMIZATION) -c $^ $(STD_LIBS) -o $(OBJ_NAME)

test: build
	$(CC) $(CFLAGS) $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test

test_ox: build_ox
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test

test_E:
	$(CC) $(CFLAGS) -E $(TEST_SRC) $(SRC)

run_test: test
	./out/test

run_test_ox: test_ox
	./out/test
