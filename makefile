CC = gcc
srcServer =	$(wildcard libs/*.c) \
			$(wildcard src/server/*.c)
srcClient = $(wildcard libs/*.c) \
			$(wildcard src/client/*.c)
srcMainClient = $(wildcard libs/*.c) \
			$(wildcard src/mainClient/*.c)
serverObj = $(srcServer:.c=.o)
clientObj = $(srcClient:.c=.o)
mainClientObj = $(srcMainClient:.c=.o)
include = -I./libs

all : clean bin/client bin/server bin/mainClient

bin/client : $(clientObj)
	$(CC) -o $@ $^ $(include) -lncurses -pthread
	
bin/server : $(serverObj)
	$(CC) -o $@ $^ $(include) -lncurses -pthread

bin/mainClient :
	$(CC) -o $@ src/mainClient/mainClient.c

.PHONY: clean

clean :
	rm -rf */*.o 
