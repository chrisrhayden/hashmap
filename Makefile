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

build_perftools: $(SRC)
	$(CC) $(CFLAGS) $(OPTIMIZATION) -c $^ $(STD_LIBS) -lprofiler -o $(OBJ_NAME)

build_pg: $(SRC)
	$(CC) $(CFLAGS) -pg -c $^ $(STD_LIBS) -o $(OBJ_NAME)
	# $(CC) $(CFLAGS) $(OPTIMIZATION) -pg -c $^ $(STD_LIBS) -o $(OBJ_NAME)

test: build
	$(CC) $(CFLAGS) $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test

test_ox: build_ox
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test

test_perftools: build_perftools
	$(CC) $(CFLAGS) $(OPTIMIZATION) $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -lprofiler -o ./out/test

test_pg: build_pg
	$(CC) $(CFLAGS) -pg $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test
	# $(CC) $(CFLAGS) $(OPTIMIZATION) -pg $(TEST_SRC) $(OBJ_NAME) $(STD_LIBS) -o ./out/test

test_E:
	$(CC) $(CFLAGS) -E $(TEST_SRC) $(SRC)

run_test: test
	./out/test

run_test_ox: test_ox
	./out/test

run_test_pg: test_pg
	./out/test
	gprof ./out/test > ./out/gprof_out.stats

run_test_perf: test_ox
	perf stat ./out/test
