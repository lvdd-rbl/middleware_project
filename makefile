CC = gcc
srcServer =	$(wildcard libs/*.c) \
			$(wildcard src/server/*.c)
srcClient = $(wildcard libs/*.c) \
			$(wildcard src/client/*.c)
serverObj = $(srcServer:.c=.o)
clientObj = $(srcClient:.c=.o)
include = -I./libs

all : bin/client bin/server

bin/client : $(clientObj)
	$(CC) -o $@ $^ $(include) -lncurses -pthread
	
bin/server : $(serverObj)
	$(CC) -o $@ $^ $(include) -lncurses -pthread

.PHONY: clean

clean :
	rm -rf */*.o 
