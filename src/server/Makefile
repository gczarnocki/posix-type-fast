CC=gcc
H_PATH=../../headers
H_DEFS=${H_PATH}/common.h ${H_PATH}/io.h ${H_PATH}/signals.h ${H_PATH}/sockets.h
C_FLAGS=-Wall -Werror -pedantic -g -I${H_PATH}

TARGET=server

.PHONY: all clean

all: ${TARGET}

# main target 

${TARGET}: server.o common.o
	gcc server.o common.o -o ${TARGET}

# compiled files

server.o: server.c ${H_DEFS}
	gcc -c server.c -o server.o ${C_FLAGS}

# header files

common.o: ${H_PATH}/common.c ${H_PATH}/common.h
	gcc -c ${H_PATH}/common.c -o common.o ${C_FLAGS}

io.o: ${H_PATH}/io.c ${H_PATH}/io.h
	gcc -c ${H_PATH}/io.c -o io.o ${C_FLAGS}

signals.o: ${H_PATH}/signals.c ${H_PATH}/signals.h
	gcc -c ${H_PATH}/signals.c -o signals.o ${C_FLAGS}

sockets.o: ${H_PATH}/sockets.c ${H_PATH}/sockets.h
	gcc -c ${H_PATH}/sockets.c -o sockets.o ${C_FLAGS}

# cleaning

clean:
	-rm ${TARGET} *.o ${H_PATH}/*.o
