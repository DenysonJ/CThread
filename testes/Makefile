#	Makefile
#
#	para os testes
#

CC=gcc
LIB_DIR=../lib
BIN_DIR=../bin

all: cidentify cyield cyield_co semaphore

cidentify: cidentify.c $(LIB_DIR)/libcthread.a
	$(CC) -o tcident cidentify.c -Wall $(LIB_DIR)/libcthread.a && mv tcident $(BIN_DIR)

cyield: cyield.c $(LIB_DIR)/libcthread.a
	$(CC) -o tcyield cyield.c -Wall $(LIB_DIR)/libcthread.a && mv tcyield $(BIN_DIR)

cyield_co: cyield_complex.c $(LIB_DIR)/libcthread.a
	$(CC) -o tcyield_co cyield_complex.c -Wall $(LIB_DIR)/libcthread.a && mv tcyield_co $(BIN_DIR)

semaphore: semaphore.c $(LIB_DIR)/libcthread.a
	$(CC) -o tsemaphore semaphore.c -Wall $(LIB_DIR)/libcthread.a && mv tsemaphore $(BIN_DIR)

cjoin: cjoin.c $(LIB_DIR)/libcthread.a
	$(CC) -o tcjoin cjoin.c -Wall $(LIB_DIR)/libcthread.a && mv tcjoin $(BIN_DIR)

cjoin_co: cjoin_complex.c $(LIB_DIR)/libcthread.a
	$(CC) -o tcjoin_co cjoin_complex.c -Wall $(LIB_DIR)/libcthread.a && mv tcjoin_co $(BIN_DIR)
