#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	char question[200];
	char options[4][50];
	int answer;
} question;


int main(){
	question questions[20];
	int i=0;
	FILE* file = NULL;
	if((file = fopen("../src/questions/questions", "r")) != NULL){
		while(fscanf(file, "%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n%i\n", questions[i].question, questions[i].options[0], questions[i].options[1], questions[i].options[2], questions[i].options[3], &questions[i].answer) != EOF){
			char * pch;
			while((pch = strstr(questions[i].question, "|")) != NULL){
				*pch = 10;
			}
			printf("%s\n%s | %s | %s | %s\nReponse : %i\n", questions[i].question, questions[i].options[0], questions[i].options[1], questions[i].options[2], questions[i].options[3], questions[i].answer);
			i++;
		}
	}
	else 
		printf("Can't open file\n");
	return 0;
}
