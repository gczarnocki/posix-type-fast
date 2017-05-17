CC=gcc
H_DEFS=common.h common_task.h
O_DEFS=common.o common_task.o

C_FLAGS=-Wall -Werror -pedantic -g
L_FLAGS=-lrt -lpthread

SERVER=server

.PHONY: all clean

all: ${SERVER}

# main target 

${SERVER}: server.o ${O_DEFS}
	gcc server.o ${O_DEFS} -o ${SERVER} ${L_FLAGS}

# compiled files

server.o: server.c ${H_DEFS}
	gcc -c server.c -o server.o ${C_FLAGS}

common.o: common.c common.h
	gcc -c common.c -o common.o ${C_FLAGS}

common_task.o: common_task.c common_task.h
	gcc -c common_task.c -o common_task.o ${C_FLAGS}

# cleaning

clean:
	-rm ${SERVER} *.o *.h.gch common common_task
