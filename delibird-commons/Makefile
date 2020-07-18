CC = gcc
CFLAGS = -g -Wall
LIBS = -lcommons
HEADERS = $(shell find delibird-commons -name "*.h")
SOURCES = $(shell find delibird-commons -name "*.c")
OBJECTS = $(patsubst %.c, build/%.o, $(SOURCES))


all: delibird-commons

delibird-commons: create-build-dir $(OBJECTS)
	$(CC) $(CFLAGS) -shared -o build/libdelibird-commons.so $(OBJECTS) $(LIBS)

create-build-dir:
	mkdir -p build/delibird-commons/model
	mkdir -p build/delibird-commons/test
	mkdir -p build/delibird-commons/utils

build/%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -rf build
