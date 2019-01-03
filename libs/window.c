#include "window.h"
/**
  \file window.c
  \author Lucas VAN DEN DRIESSCHE
  \version v1.0
  \brief Librairie contenant des fonctions permettant de gérer les fonctionnalités de Ncurses.
  \date 31 décembre 2018
*/

/**
  \fn int drawAll(char *pseudo, int nbQuestions)
  \param pseudo --> le pseudo du joueur qui sera affiché dans les fenêtres
  \param nbQuestions --> le nombre de questions à afficher
  \brief Fonction permettant d'appeler les différentes fonctions d'initialisation de fenêtres
  \return int --> le choix que l'utilisateur a sélectionné dans le menu d'accueil
*/
int drawAll(char *pseudo, int nbQuestions) {
	int choice = 0;
	pseudoUser = pseudo;
	
	drawBackground();
	drawTitle();
	drawStats(pseudo,nbQuestions);
	drawQuizTitle();
	choice = printMenu();
	return choice;	
}

/**
  \fn void drawBackground()
  \brief Dessine un arrière-plan sur les dimensions de départ données
  \return void
*/
void drawBackground() {
	erase();
   	for (int i=0; i<lines; i++) {
   		wattrset(stdscr,COLOR_PAIR(4));
    	mvhline(i, 0, ACS_CKBOARD,cols);
    	wattrset(stdscr,0);
   	}
   	wnoutrefresh(stdscr);
}

/**
  \fn void drawLobby()
  \brief Dessine l'écran du lobby dans la fenêtre "game"
  \return void
*/
void drawLobby() {
	int y,x;
	getmaxyx(game,y,x);
	wattrset(game,COLOR_PAIR(1) | A_BOLD);
	mvwprintw(game,6,(x-strlen("En attente d'autres joueurs pendant 15 secondes"))/2,
						"En attente d'autres joueurs pendant 15 secondes");
	mvwprintw(game,8,(x-strlen("Joueurs présents :"))/2,"Joueurs présents :");
	wattrset(game,COLOR_PAIR(3) | A_BOLD);
	mvwprintw(game,10,(x-strlen(pseudoUser))/2,pseudoUser);
	wrefresh(game);
}

/**
  \fn void drawQuizTitle()
  \brief Dessine le titre du quiz dans la fenêtre "game"
  \return void
*/
void drawQuizTitle() {
	int y,x;
   	werase(game);
   	wattrset(game,0);
   	box(game,0,0);
   	getmaxyx(game,y,x);
   	wattrset(game,A_BOLD | COLOR_PAIR(4));
   	mvwprintw(game,2,(x-strlen("Quiz"))/2,"Quiz");
   	mvwhline(game,4,2,ACS_HLINE,x-4);	
   	wrefresh(game);	
}

/**
  \fn void drawStats(char *pseudo, int nbQuestions)
  \param pseudo --> le pseudo du joueur qui sera affiché dans les fenêtres
  \param nbQuestions --> le nombre de questions à afficher
  \brief Dessine la fenêtre des statistiques qui affichera certaines données
  	utiles pour le joueur : son pseudo, le nombre de questions, son score...
  \return void
*/
void drawStats(char *pseudo, int nbQuestions) {
	int y,x;
	werase(stats);
   	box(stats,0,0);
   	getmaxyx(stats,y,x);
   	wattrset(stats,A_BOLD | COLOR_PAIR(4));
   	mvwprintw(stats,2,(x-strlen("Statistiques"))/2,
   					"Statistiques");
   	mvwhline(stats,4,2,ACS_HLINE,cols/3 - 5);
   	wattrset(stats,COLOR_PAIR(1));
   	mvwprintw(stats,6,(x-strlen("Joueur : %s"))/2,
   					"Joueur : %s",pseudo);
   	mvwprintw(stats,8,(x-strlen("Difficulté : Facile"))/2,
   					"Difficulté : Facile");
   	mvwprintw(stats,10,(x-strlen("Nombre de questions : %d"))/2,
   					"Nombre de questions : %d",nbQuestions); 
   	mvwprintw(stats,12,(x-strlen("Restantes : %d"))/2,
   					"Restantes : %d",nbQuestions);
   	mvwprintw(stats,14,(x-strlen("Score actuel : 0"))/2,
   					"Score actuel : 0");
   	wnoutrefresh(stats);
}

/**
  \fn void drawTitle()
  \brief Dessine le titre du jeu sur la partie supérieure du terminal
  \return void
*/
void drawTitle() {
	werase(title);
   	wattrset(title,COLOR_PAIR(3) | A_BOLD);
   	mvwaddstr(title, 0, (cols-strlen("Bienvenue sur nquiz : un projet MCS"))/2, 
   		"Bienvenue sur nquiz : un projet MCS");
   	wattrset(title,0);
   	wnoutrefresh(title);
}

/**
  \fn void initGraphics()
  \brief Initialise la librairie Ncurses
  \return void
*/
void initGraphics() {
	initscr(); // Entre dans le GUI
	clear(); 
	noecho(); // Pas d'écho des caractères typés 
	cbreak(); // Pas de line buffering 	
	curs_set(0); // Pas de curseur

	if(has_colors() == FALSE) {	
		endwin();
		printf("Votre terminal ne supporte pas les couleurs.\n");
		exit(1);
	}
	start_color(); // Mise en place d'un set de couleurs			
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_RED, COLOR_BLACK);
	init_pair(4, COLOR_BLUE, COLOR_BLACK);
	getmaxyx(stdscr,lines,cols); // lines et cols sont les dimensions max de l'écran principal
	refresh();
}

/**
  \fn void initWindows()
  \brief Initialise les différentes fenêtres du jeu avec des coordonnées
  	et des dimensions données
  \return void
*/
void initWindows() {
	title = newwin(1,cols - 2,0,1);
	stats = newwin(lines - 4,cols/3 - 1,3,1);
	game = newwin(lines - 4,2*cols/3 - 3,3,cols/3 + 3);
}

/**
  \fn int printMenu()
  \brief Affiche le menu d'accueil avec les différents choix
  	Attend un choix de l'utilisateur avec l'utilisation de touches du clavier
  	(flèches du haut et du bas) et renvoie le choix de l'utilisateur
  \return int --> le choix de l'utilisateur
*/
int printMenu() {
	int highlight = 1;
	int choice = 0;
	int c,y,x;
	getmaxyx(game,y,x); // y et x sont la hauteur et la largeur de la fenêtre game
	choices[0] = "Créer une partie";
	choices[1] = "Rejoindre une partie";
	choices[2] = "Quitter le jeu";

	printOptions(game,highlight,choices,3,6);
	wattrset(game,COLOR_PAIR(4) | A_BOLD);
	mvwhline(game,13,2,ACS_HLINE,x-4);
	wattrset(game,COLOR_PAIR(1));
   	mvwprintw(game,16,(x-strlen("Les règles sont les suivantes :"))/2,
   						"Les règles sont les suivantes :");
   	mvwprintw(game,17,(x-strlen("- Chaque question possède un délai de 30s"))/2,
   						"- Chaque question possède un délai de 30s");
   	mvwprintw(game,18,(x-strlen("- Pour buzzer, appuyez sur Espace"))/2,
   						"- Pour buzzer, appuyez sur Espace");
   	mvwprintw(game,19,(x-strlen("- Vous aurez alors 5 secondes pour répondre"))/2,
   						"- Vous aurez alors 5 secondes pour répondre");
   	mvwprintw(game,21,(x-strlen("Bon jeu."))/2,
   						"Bon jeu.");

   	keypad(game, TRUE); // accepte les combinaisons spéciales au clavier
   	while(1) {	
   		c = wgetch(game);
		switch(c) {	
			case KEY_UP:
				if(highlight == 1)
					highlight = 3;
				else
					highlight--;
				break;
			case KEY_DOWN:
				if(highlight == 3)
					highlight = 1;
				else 
					highlight++;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				// mvwprintw(game,2,2,"Touche : %c",c);
				// wrefresh(game);
				break;
		}
		printOptions(game,highlight,choices,3,6);

		// Si l'utilisateur a fait son choix
		if(choice != 0)
			break;
	}
   	return choice;
}

/**
  \fn void printOptions(WINDOW *win, int highlight, char *options[], int nOptions, int baseY)
  \param WINDOW *win --> la fenêtre sur laquelle les options seront affichées
  \param int highlight --> le numéro de l'option à mettre en avant
  \param char *options[] --> le tableau de chaînes de caractères à afficher dans les options
  \param int nOptions --> le nombre d'options à afficher
  \param int baseY --> la hauteur de départ où les options seront affichées
  \brief Affiche les options passées en paramètre à partir d'une hauteur donnée
  	et met en avant l'option désignée par le paramètre highlight grâce à l'attribut
  	A_REVERSE (Reverse video)
  \return void
*/
void printOptions(WINDOW *win, int highlight, char *options[], int nOptions, int baseY) {
	int x, y, baseX;
	getmaxyx(win,y,x);	
	// wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
	
	for(int i = 0; i < nOptions; i++) {
		baseX = (x - strlen(options[i]))/2; 	
		if(highlight == i + 1) {	
			wattron(win, A_REVERSE | A_BOLD | COLOR_PAIR(1)); 
			mvwprintw(win, baseY, baseX, "%s", options[i]);
			wattroff(win, A_REVERSE | A_BOLD | COLOR_PAIR(1));
		}
		else {
			wattron(win,A_BOLD | COLOR_PAIR(1)); 
			mvwprintw(win, baseY, baseX, "%s", options[i]);
			wattroff(win,A_BOLD | COLOR_PAIR(1));
		}
		baseY+=2;
	}
	wrefresh(win);
}

/**
  \fn int printString(int y, int x, char *string)
  \param int y --> la hauteur de la fenêtre
  \param int x --> la largeur de la fenêtre
  \param char *string --> la chaîne de caractères à afficher
  \brief Permet d'afficher une chaîne de caractères
  	comportant des retours chariot que Ncurses prend mal en charge
  	au milieu de la fenêtre game à partir de coordonnées données
	
	La fonction décompose la chaine en sous-chaines sans '\n', les affiche
	une par une et met à jour la hauteur de l'affichage y
  \return int y --> la hauteur de la prochaine ligne
*/
int printString(int y, int x, char *string) {
	char *n = malloc(strlen(string?string:"\n"));
	char *token;
	if(string)
		strcpy(n,string);
	n[strlen(n)-1] ='\0';
	token = strtok(n,"\n");
	while(token != NULL) {
		mvwprintw(game,y,(x-strlen(token))/2,"%s",token);
		wrefresh(game);
		token = strtok(NULL,"\n");
		y+=2;
	}
	return y;
}