#include "serialize.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#define CHECK(sts,msg) if((sts)==-1){perror(msg);exit(-1);}
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#include <fcntl.h>
#include <semaphore.h>

#include <error.h>

#define BUFFER_SIZE 1024
#define SVC_PORT  5000
#define DEFAULT_QUEUE 5 
#define CONTINUE(value) value = value - 1; if((value) <= 0) { continue; }

char buffer[BUFFER_SIZE];

typedef struct{
	int s;
	struct sockaddr_in clt;
}	socketInfo_t;

typedef struct {
	int fd;
	int score;
	char *pseudo;
}	userInfo;


#define MAX_USERS 3
#define NB_QUESTIONS 20
#define ANSWER_TIME 5
#define QUESTION_TIME 10
#define LOGIN_WAIT 15
#define LOBBY_STATE 0
#define GAME_STATE 1

int nbUsers;
int timer;
int timerAnswer;
userInfo users[MAX_USERS];
sem_t *buzzer;
int serverCanAccept;
int serverMode;

void assignResources(socketInfo_t * sockInfo, int listenSocket, void (*queryHandler)(socketInfo_t *, int));
void bindAddress(int *socket, struct sockaddr_in sockAddr);
void computeAddress(struct sockaddr_in * addr, char * address);
void connectToServer(int s, char * address);
void deroute(int numSignal);
void initSignaux(void);
int getSockType(int s);
void listenMode(int s, int queue);
int receiveDatas(query * query, socketInfo_t * sockInfo, int stream);
void socketInit(int *s, int type, char * address, int flag);
void waitForClient(socketInfo_t * sockInfo, int listenSocket, void (*f)(socketInfo_t *, int));
void writeDatas(query * query, socketInfo_t * sockInfo, int s);


