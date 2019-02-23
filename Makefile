CC=clang
CFLAGS=-Wall -Wno-pragma-pack
LDFLAGS=-L.
BINARY=networking

ifeq ($(OS),Windows_NT)
	BINARY:=$(BINARY).exe
endif

all: networking

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

networking: main.o
	$(CC) -o $(BINARY) $(LDFLAGS) $^

networking2: newmain.o dictionary.o queue.o
	$(CC) -o $(BINARY) $(LDFLAGS) $^

# gpu: gpu.o
# 	$(CC) -o $(BINARY_GPU) $(LDFLAGS) $^

# plugin.dll: plugin.c
# 	$(CC) -o $@ -shared $^