#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 1024

typedef struct{
		int id;
		char * query;
		char *info;
} query;

typedef struct{
		int id;
		char * answer;
} answer;

char * queryToString(query *);
query stringToQuery(char *);

char * answerToString(answer *);
answer stringToAnswer(char *);

