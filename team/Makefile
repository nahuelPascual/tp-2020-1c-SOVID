CC = gcc
CFLAGS = -g -Wall
LIBS = -ldelibird-commons -lcommons -lpthread -lm
HEADERS = $(shell find src -name "*.h")
SOURCES = $(shell find src -name "*.c")
OBJECTS = $(patsubst %.c, build/%.o, $(SOURCES))


all: team

team: create-build-dir $(OBJECTS) ../delibird-commons/build/libdelibird-commons.so
	$(CC) $(CFLAGS) -L../delibird-commons/build -o build/team $(OBJECTS) $(LIBS)

create-build-dir:
	mkdir -p build/src/test

build/%.o: %.c
	$(CC) $(CFLAGS) -I../delibird-commons -c $< -o $@

clean:
	rm -rf build
