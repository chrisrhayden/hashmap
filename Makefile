CC=gcc
CFLAGS=-Wall -ggdb3 -pedantic

SRC = $(wildcard ./src/*.c)

TARGET_NAME = ./out/target_build

LIBS=-L./libs/ -lspookyhash

build: $(SRC)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $(TARGET_NAME)

run: build
	./out/target_build
