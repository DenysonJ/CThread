#	Makefile
#
#	para os testes
#

CC=gcc
LIB_DIR=../lib
BIN_DIR=../bin

all: cidentify cyield cyield_co semaphore

cidentify: cidentify.c $(LIB_DIR)/libcthread.a
	$(CC) -o cident cidentify.c -Wall $(LIB_DIR)/libcthread.a && mv cident $(BIN_DIR)

cyield: cyield.c $(LIB_DIR)/libcthread.a
	$(CC) -o cyield cyield.c -Wall $(LIB_DIR)/libcthread.a && mv cyield $(BIN_DIR)

cyield_co: cyield_complex.c $(LIB_DIR)/libcthread.a
	$(CC) -o cyield_co cyield_complex.c -Wall $(LIB_DIR)/libcthread.a && mv cyield_co $(BIN_DIR)

semaphore: semaphore.c $(LIB_DIR)/libcthread.a
	$(CC) -o semaphore semaphore.c -Wall $(LIB_DIR)/libcthread.a && mv semaphore $(BIN_DIR)

cjoin: cjoin.c $(LIB_DIR)/libcthread.a
	$(CC) -o cjoin cjoin.c -Wall $(LIB_DIR)/libcthread.a && mv cjoin $(BIN_DIR)

cjoin_co: cjoin_complex.c $(LIB_DIR)/libcthread.a
	$(CC) -o cjoin_co cjoin_complex.c -Wall $(LIB_DIR)/libcthread.a && mv cjoin_co $(BIN_DIR)
