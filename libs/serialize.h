#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define BUFFER_SIZE 1024

typedef struct {
	char question[200];
	char options[4][50];
	int answer;
} question;


typedef struct{
		int id;
		char * query;
		question quest;
} query;

typedef struct{
		int id;
		char * answer;
} answer;

char * queryToString(query *);
query stringToQuery(char *);

char * answerToString(answer *);
answer stringToAnswer(char *);

