BIN_DIR=bin
SRC_DIR=src

.PHONY: all clean

all: client server
	cp ${SRC_DIR}/client/client ${BIN_DIR}/
	cp ${SRC_DIR}/server/server ${BIN_DIR}/

client:
	make -C ${SRC_DIR}/client

server:
	make -C ${SRC_DIR}/server

clean:
	make clean -C ${SRC_DIR}/client
	make clean -C ${SRC_DIR}/server
	-rm ${BIN_DIR}/client ${BIN_DIR}/server
