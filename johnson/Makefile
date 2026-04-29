CC = gcc

CFLAGS = -Iinclude/

LDFLAGS = -lOpenCL

WFLAGS = \
	-Wall \
	-Wextra \
	-Wpedantic

SRC = \
	src/johnson.c \
	src/opencl.c \
	main.c

OUT = main

all:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) $(WFLAGS) -o $(OUT)
