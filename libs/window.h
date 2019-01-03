#include <stdio.h>
#include <ncurses.h>

int cols,lines;
int nOptions;
int score;
char *pseudoUser;
char *choices[3];
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
void printOptions(WINDOW *, int,char *[], int, int);
int printString(int, int, char *);