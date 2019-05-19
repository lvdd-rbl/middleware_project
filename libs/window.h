#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <error.h>

int cols,lines;
int nOptions;
int score;
char *pseudoUser;
WINDOW *title, *stats, *game;

int drawAll(char *,int);
void drawBackground(void);
void drawLobby(void);
void drawQuizTitle(void);
void drawStats(char *,int);
void drawTitle(void);
void initGraphics(void);
void initWindows(void);
int printMenu(void);
void printOptions(WINDOW *, int,char [4][50], int, int);
int printString(int, int, char *);
