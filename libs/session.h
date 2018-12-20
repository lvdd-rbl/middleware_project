#include "serialize.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#define CHECK(sts,msg) if((sts)==-1){perror(msg);exit(-1);}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define SVC_PORT  5000
#define DEFAULT_QUEUE 5 
#define CONTINUE(value) value = value - 1; if((value) <= 0) { continue; }

char buffer[BUFFER_SIZE];

typedef struct{
	int s;
	struct sockaddr_in clt;
}	socketInfo_t;

void assignResources(socketInfo_t * sockInfo, int listenSocket, void (*queryHandler)(socketInfo_t *));
void bindAddress(int *socket, struct sockaddr_in sockAddr);
void computeAddress(struct sockaddr_in * addr, char * address);
void deroute(int numSignal);
void finishChildren(void);
int getSockType(int s);
void listenMode(int s, int queue);
void socketInit(int *s, int type, char * address, int flag);
void waitForClient(socketInfo_t * sockInfo, int listenSocket, void (*f)(socketInfo_t *));
void writeDatas(query * query, socketInfo_t * sockInfo, int s);
int receiveDatas(query * query, socketInfo_t * sockInfo, int stream);

