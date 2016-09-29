#
# Makefile para programas exemplo
# 

CC=gcc
LIB_DIR=../lib

all: cyield  cidentify  semaphore

cyield: cyield.c $(LIB_DIR)/libcthread.a
	$(CC) -o cyield cyield.c $(LIB_DIR)/libcthread.a -Wall

cidentify: cidentify.c $(LIB_DIR)/libcthread.a
	$(CC) -o cidentify cidentify.c $(LIB_DIR)/libcthread.a -Wall

semaphore: semaphore.c $(LIB_DIR)/libcthread.a
	$(CC) -o semaphore semaphore.c $(LIB_DIR)/libcthread.a -Wall

clean:
	rm -rf cyield  cidentify  semaphore *.o *~
