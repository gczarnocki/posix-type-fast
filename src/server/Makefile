CC=gcc
H_PATH=../../headers
C_FLAGS=-Wall -Werror -pedantic -g -I${H_PATH}

TARGET=server

.PHONY: all clean

all: ${TARGET}

# main target 

${TARGET}: server.o ${H_PATH}/common.o
	gcc server.o ${H_PATH}/common.o -o ${TARGET}

# compiled files

server.o: server.c ${H_PATH}/common.h
	gcc -c server.c -o server.o ${C_FLAGS}

common.o: ${H_PATH}/common.c ${H_PATH}/common.h
	gcc -c ${H_PATH}/common.c -o ${H_PATH}/common.o ${C_FLAGS}

clean:
	-rm ${TARGET} *.o ${H_PATH}/*.o
