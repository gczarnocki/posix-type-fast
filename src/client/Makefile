CC=gcc
H_PATH=../../headers
C_FLAGS=-Wall -Werror -pedantic -g -I${H_PATH}

TARGET=client

.PHONY: all clean

all: ${TARGET}

# main target 

${TARGET}: client.o ${H_PATH}/common.o
	gcc client.o ${H_PATH}/common.o -o ${TARGET}

# compiled files

client.o: client.c ${H_PATH}/common.h
	gcc -c client.c -o client.o ${C_FLAGS}

common.o: ${H_PATH}/common.c ${H_PATH}/common.h
	gcc -c ${H_PATH}/common.c -o ${H_PATH}/common.o ${C_FLAGS}

# cleaning

clean:
	-rm ${TARGET} *.o ${H_PATH}/*.o
