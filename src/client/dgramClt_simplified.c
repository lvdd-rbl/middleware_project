#include "../../libs/session.h"
#include "../../libs/window.h"

/**
  \file dgramClt_simplified.c
  \author SUEUR Samuel & VAN DEN DRIESSCHE Lucas
  \version v1.0
  \brief Fichier principal pour le client.
  \date 31 décembre 2018
*/

void lobby(int,int,char*,int, char* [], query*);
/**
	\fn void lobby(int s, int choice, char *pseudo, int argc, char* argv[], query* dataStructure)
	\param s --> la socket sur laquelle on souhaite la connexion
	\param choice --> le choix de l'utilisateur dans le menu d'accueil
	\param pseudo --> le pseudo de l'utilisateur
	\param argc --> le parametre argc du main
	\param argv --> le parametre argv du main, necessaire pour la connexion a un serveur distant
	\param dataStructure --> la structure utilisee pour communiquer avec le serveur
	\brief Fonction qui permet la gestion du lobby par le joueur
	\return void
*/
void lobby(int s, int choice, char *pseudo, int argc, char* argv[], query * dataStructure) {
	char *chaine = malloc(BUFFER_SIZE);
	int baseY=12,y,x;
	drawQuizTitle();
	if(argc>1){
		connectToServer(s, argv[1]);
	}else{
		connectToServer(s, "127.0.0.1:5000");
	}
	sprintf(chaine,"%d",choice);
	strcat(chaine,":");
	strcat(chaine,pseudo);
	strcat(chaine, ":0:0:0:0:0:0");
	*dataStructure = stringToQuery(chaine);
	// Envoie 1:pseudo ou 2:pseudo pour créer ou
	// rejoindre une partie
	writeDatas(dataStructure,NULL,s); //envoi choix & pseudo
	receiveDatas(dataStructure,NULL,s); //ACK du serveur
	if(choice == 1){
		if(dataStructure->id == 101){ //Si le serveur a créé une partie
			dataStructure->id = 10; //On demande au serveur d'attendre les joueurs
			strcpy(dataStructure->query, "15"); //On définit l'alarme à 15s
			writeDatas(dataStructure,NULL,s); //Envoi requête
			receiveDatas(dataStructure,NULL,s); //ACK du serveur	
		}
	}
	drawLobby();
	wattrset(game,COLOR_PAIR(3) | A_BOLD);
	while(dataStructure->id != -10) { //Phase d'attente du lobby
		receiveDatas(dataStructure, NULL, s);

		// Affiche les pseudos des autres joueurs, s'il y a
		if(dataStructure->id != -10) { //Tant que le serveur ne signale pas la fin du lobby
			getmaxyx(game,y,x);
			mvwprintw(game,baseY,(x-strlen(dataStructure->query))/2,
								dataStructure->query);
			baseY+=2;
			wrefresh(game);
		}
	}
}

int main(int argc, char * argv[]){
	int s, nb, nbBits, choice = 0, baseY=6, y, x;
	query dataStructure;
	CHECK(buzzer = sem_open("/monBuzzer", O_CREAT|O_EXCL, 0666,1),"Probleme sem_open");
	socketInit(&s, SOCK_STREAM, NULL, 0);
	serverMode=1;
	char pseudo[8];
	printf("Pseudo : ");
	// fgets(pseudo,8,stdin);
	scanf("%[^\n]",pseudo);
	pseudo[8] = '\0';

	initGraphics();
	initWindows();
	initSignaux(0);
	choice = drawAll(pseudo,NB_QUESTIONS);

   	switch(choice) {
   		case 3 : // Quitter le jeu
   			endwin();
   			exit(1);
   			break;
   		default : // Créer une nouvelle partie ou rejoindre une nouvelle partie
   			lobby(s, choice, pseudo, argc, argv, &dataStructure);
   			break;
   	}

   	werase(game);
   	drawQuizTitle();
   	getmaxyx(game,y,x);
   	wattrset(game,COLOR_PAIR(1) | A_BOLD);
   	mvwprintw(game,6,(x-strlen("Le jeu va commencer dans 2 secondes"))/2,
   						"Le jeu va commencer dans 2 secondes");
   	wrefresh(game);
   	sleep(2);


	werase(game);
   	drawQuizTitle();
	int highlight = 0, score = 0, index, c, nbQuestions = NB_QUESTIONS;
	keypad(game,TRUE); // accepte les combinaisons au clavier
	double timeLeft;
	time_t time1, time2;
	
	for(int i = 0; i<NB_QUESTIONS; i++){
		dataStructure.id = 20;
		writeDatas(&dataStructure, NULL, s); //Envoi demande question serveur
		receiveDatas(&dataStructure, NULL,s);
		if(dataStructure.id != 120) exit(EXIT_FAILURE);
		
		werase(game);
		drawQuizTitle();
		baseY = 6;
		choice = 0;
		highlight = 0;
		getmaxyx(game,y,x);
		wattrset(game,COLOR_PAIR(1) | A_BOLD | A_UNDERLINE);
		mvwprintw(game,baseY,(x-strlen("Question %d"))/2,"Question %d",i+1);
		wattrset(game,COLOR_PAIR(4) | A_BOLD);
		baseY = printString(baseY+2,x,dataStructure.quest.question);
		printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
		box(game,0,0);
		wrefresh(game);
		
		flushinp(); // important
		wtimeout(game,10000); 
		c = wgetch(game);
		switch(c) {
			case 32 :
				// TODO - Un buzzer par client par question //////////////////////////////
				// CHECK(sem_trywait(buzzer),"Probleme sem_wait");
				highlight = 1;
				//printOptions(game,highlight,questions[index].options,4,baseY+1);
				printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
				alarm(ANSWER_TIME); // même s'il reste moins de 5s avant la fin du temps restant
				timerAnswer = 1;
				while(timerAnswer) {
					c = wgetch(game);

					switch(c) {	
						case KEY_UP:
							if(highlight == 1)
								highlight = 4;
							else
								highlight--;
							break;
						case KEY_DOWN:
							if(highlight == 4)
								highlight = 1;
							else 
								highlight++;
							break;
						case 10:
							choice = highlight;
							break;
						default:
							// mvwprintw(game,3,2,"Touche : %d ou %c",c,c);
							// wrefresh(game);

							break;
					}
					printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
					if(choice != 0) {
						alarm(0); // annule l'alarme
						break;
					}
				}
				highlight = 0;
				printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
				break;
			default :
				// mvwprintw(game,2,2,"%d ou %c",c,c);
				// wrefresh(game);
				break;
		}
		// Décrémente les questions restantes dans la fenêtre stats
		wattrset(stats,COLOR_PAIR(1));
		wmove(stats,12,0);
		wclrtoeol(stats); // clear to end of line
		getmaxyx(stats,y,x);
		mvwprintw(stats,12,(x-strlen("Restantes : %d"))/2,

					"Restantes : %d",--nbQuestions);
		getmaxyx(game,y,x);

		// Si l'utilisateur a choisi une réponse
		if(choice != 0) {
			wattrset(game, COLOR_PAIR(4) | A_BOLD);
			//if(questions[index].answer == choice) {
			if(dataStructure.quest.answer == choice) {
				score++;
				mvwprintw(game,y-3,(x-strlen("Bonne réponse !"))/2,
							"Bonne réponse !");
				wmove(stats,14,0);
				wclrtoeol(stats);
				getmaxyx(stats,y,x);
				mvwprintw(stats,14,(x-strlen("Score actuel : %d"))/2,
							"Score actuel : %d",score);
				dataStructure.id = 30; //Signaler au serveur qu'on a la bonne réponse
				writeDatas(&dataStructure,NULL,s);
			} else {
				mvwprintw(game,y-3,(x-strlen("Mauvaise réponse !"))/2,
							"Mauvaise réponse !");
			}
			wrefresh(game);
		}
		wattrset(game,0);
		box(stats,0,0);

		wrefresh(stats);
		
		
	
	}
	dataStructure.id = 40; //Attendre le score final
	writeDatas(&dataStructure, NULL, s);
	receiveDatas(&dataStructure, NULL, s);
	dataStructure.id = 50; //Demander la déconnexion
	writeDatas(&dataStructure, NULL, s);
	werase(game);
 	drawQuizTitle();
 	getmaxyx(game,y,x);
 	wattrset(game,COLOR_PAIR(1) | A_BOLD); //Afficher le gagnant
 	mvwprintw(game,6,(x-strlen("Le jeu est terminé."))/2,
 						"Le jeu est terminé.");
 	mvwprintw(game,8,(x-strlen("Le gagnant est %s !!!"))/2,
 						"Le gagnant est %s !!!",dataStructure.query);
 	wrefresh(game);

 	close(s);
 	sem_close(buzzer);
 	sem_unlink("/monBuzzer");

	getch();
	endwin();
	/*do {
		receiveDatas(&dataStructure,NULL,s);
		if(dataStructure.id != -1) {
			werase(game);
   			drawQuizTitle();
   			baseY = 6;
   			choice = 0;
			index = dataStructure.id;
			highlight = 0;
			getmaxyx(game,y,x);
			wattrset(game,COLOR_PAIR(1) | A_BOLD | A_UNDERLINE);
			mvwprintw(game,baseY,(x-strlen("Question %d"))/2,"Question %d",dataStructure.id+1);
			wattrset(game,COLOR_PAIR(4) | A_BOLD);
			//baseY = printString(baseY+2,x,questions[dataStructure.id].question);
			baseY = printString(baseY+2,x,dataStructure.quest.question);
			//printOptions(game,highlight,questions[dataStructure.id].options,4,baseY+1);
			printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
			box(game,0,0);
			wrefresh(game);

			// size_t timeQues = 0;
			// Récupère le temps lors de l'envoi de la question
			// if(1 == sscanf(dataStructure.query,"%zu",&timeQues)) {
			// 	mvwprintw(game,2,2,"Temps : %zu",timeQues);
			// 	wrefresh(game);
			// }



			// man : The flushinp() routine throws away any typeahead that has been typed 
			// by the user and has not yet been read by the program.
			flushinp(); // important
			wtimeout(game,10000); 
			c = wgetch(game);

			switch(c) {
				case 32 :
					// TODO - Un buzzer par client par question //////////////////////////////
					// CHECK(sem_trywait(buzzer),"Probleme sem_wait");
					highlight = 1;
					//printOptions(game,highlight,questions[index].options,4,baseY+1);
					printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
					alarm(ANSWER_TIME); // même s'il reste moins de 5s avant la fin du temps restant
					timerAnswer = 1;
					while(timerAnswer) {
						c = wgetch(game);

						switch(c) {	
							case KEY_UP:
								if(highlight == 1)
									highlight = 4;
								else
									highlight--;
								break;
							case KEY_DOWN:
								if(highlight == 4)
									highlight = 1;
								else 
									highlight++;
								break;
							case 10:
								choice = highlight;
								break;
							default:
								// mvwprintw(game,3,2,"Touche : %d ou %c",c,c);
								// wrefresh(game);
								break;
						}
						//printOptions(game,highlight,questions[index].options,4,baseY+1);
						printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
						if(choice != 0) {
							alarm(0); // annule l'alarme
							break;
						}
					}
					highlight = 0;
					//printOptions(game,highlight,questions[index].options,4,baseY+1);
					printOptions(game,highlight,dataStructure.quest.options,4,baseY+1);
					break;
				default :
					// mvwprintw(game,2,2,"%d ou %c",c,c);
					// wrefresh(game);
					break;

			}

			// Décrémente les questions restantes dans la fenêtre stats
			wattrset(stats,COLOR_PAIR(1));
			wmove(stats,12,0);
			wclrtoeol(stats); // clear to end of line
			getmaxyx(stats,y,x);
			mvwprintw(stats,12,(x-strlen("Restantes : %d"))/2,
						"Restantes : %d",--nbQuestions);
			getmaxyx(game,y,x);

			// Si l'utilisateur a choisi une réponse
			if(choice != 0) {
				wattrset(game, COLOR_PAIR(4) | A_BOLD);
				//if(questions[index].answer == choice) {
				if(dataStructure.quest.answer == choice) {
					score++;
					mvwprintw(game,y-3,(x-strlen("Bonne réponse !"))/2,
								"Bonne réponse !");
					wmove(stats,14,0);
					wclrtoeol(stats);
					getmaxyx(stats,y,x);
					mvwprintw(stats,14,(x-strlen("Score actuel : %d"))/2,
								"Score actuel : %d",score);
								
					writeDatas(&dataStructure,NULL,s);
				} else {
					mvwprintw(game,y-3,(x-strlen("Mauvaise réponse !"))/2,
								"Mauvaise réponse !");
				}
				wrefresh(game);
			}
			wattrset(game,0);
			box(stats,0,0);
			wrefresh(stats);

		}
		// CHECK(sem_post(buzzer),"pb sem_post");
	} while(dataStructure.id != -1);*/

	/*werase(game);
   	drawQuizTitle();
   	getmaxyx(game,y,x);
   	wattrset(game,COLOR_PAIR(1) | A_BOLD);
   	mvwprintw(game,6,(x-strlen("Le jeu est terminé."))/2,
   						"Le jeu est terminé.");
   	mvwprintw(game,8,(x-strlen("Le gagnant est %s !!!"))/2,
   						"Le gagnant est %s !!!",dataStructure.query);
   	wrefresh(game);

   	close(s);
   	sem_close(buzzer);
   	sem_unlink("/monBuzzer");

	getch();
	endwin();*/

 	// serverDialogue(s);
	return 0;
}
