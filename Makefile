BIN_DIR=bin
SRC_DIR=src

.PHONY: all clean

all: server
	cp ${SRC_DIR}/server ${BIN_DIR}/

server:
	make all -C ${SRC_DIR}

clean:
	make clean -C ${SRC_DIR}
	rm ${BIN_DIR}/server
