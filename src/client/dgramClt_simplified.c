#include "../../libs/session.h"
#include "../../libs/window.h"

void lobby(int,int,char*);
void serverDialogue(int);
question* setQuestions(void);
int waitEvent(fd_set *,int);

/**
	\fn void lobby(int s, int choice, char *pseudo)
	\param s --> la socket sur laquelle on souhaite la connexion
	\param choice --> le choix de l'utilisateur dans le menu d'accueil
	\param pseudo --> le pseudo de l'utilisateur
	\brief Fonction qui permet la gestion du lobby par le joueur
	\return void
*/
void lobby(int s, int choice, char *pseudo) {
	char *chaine = malloc(BUFFER_SIZE);
	int baseY=12,y,x;
	query dataStructure;
	drawQuizTitle();
	// if(argc>1)
		// connectToServer(s, argv[1]);
	// else
	connectToServer(s, "127.0.0.1:5000");
	sprintf(chaine,"%d",choice);
	strcat(chaine,":");
	strcat(chaine,pseudo);
	dataStructure = stringToQuery(chaine);
	// Envoie 1:pseudo ou 2:pseudo pour créer ou
	// rejoindre une partie
	writeDatas(&dataStructure,NULL,s);
	drawLobby();
	wattrset(game,COLOR_PAIR(3) | A_BOLD);
	while(dataStructure.id != 0) {
		receiveDatas(&dataStructure, NULL, s);

		// Affiche les pseudos des autres joueurs, s'il y a
		if(dataStructure.id != 0) {
			getmaxyx(game,y,x);
			mvwprintw(game,baseY,(x-strlen(dataStructure.query))/2,
								dataStructure.query);
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

	char pseudo[8];
	printf("Pseudo : ");
	// fgets(pseudo,8,stdin);
	scanf("%[^\n]",pseudo);
	pseudo[8] = '\0';

	initGraphics();
	initWindows();
	choice = drawAll(pseudo,NB_QUESTIONS);

   	switch(choice) {
   		case 1 : // Créer une nouvelle partie
   			lobby(s,1,pseudo);
   			break;
   		case 2 : // Rejoindre une partie en cours
   			lobby(s,2,pseudo);
   			break;
   		case 3 : // Quitter le jeu
   			endwin();
   			exit(1);
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

   	question *questions;
   	questions = setQuestions();
	werase(game);
   	drawQuizTitle();
	int highlight = 0, score = 0, index, c, nbQuestions = NB_QUESTIONS;
	keypad(game,TRUE); // accepte les combinaisons au clavier
	double timeLeft;
	time_t time1, time2;

	do {
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
			baseY = printString(baseY+2,x,questions[dataStructure.id].question);
			printOptions(game,highlight,questions[dataStructure.id].options,4,baseY+1);
			box(game,0,0);
			wrefresh(game);

			// size_t timeQues = 0;
			// Récupère le temps lors de l'envoi de la question
			// if(1 == sscanf(dataStructure.query,"%zu",&timeQues)) {
			// 	mvwprintw(game,2,2,"Temps : %zu",timeQues);
			// 	wrefresh(game);
			// }

			initSignaux();

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
					printOptions(game,highlight,questions[index].options,4,baseY+1);
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
						printOptions(game,highlight,questions[index].options,4,baseY+1);
						if(choice != 0) {
							alarm(0); // annule l'alarme
							break;
						}
					}
					highlight = 0;
					printOptions(game,highlight,questions[index].options,4,baseY+1);
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
				if(questions[index].answer == choice) {
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
	} while(dataStructure.id != -1);

	werase(game);
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
	endwin();

 	// serverDialogue(s);
	return 0;
}

void serverDialogue(int s) {
	int nb = 1, nbBits;
	fd_set rfds;
	query dataStructure;
	while(nb != 0){
		nbBits = waitEvent(&rfds, s);
		if(FD_ISSET(0, &rfds)){
			receiveDatas(&dataStructure, NULL, 0);
			printf("Envoi de : '%i:%s'\n", dataStructure.id, dataStructure.query);
			writeDatas(&dataStructure, NULL, s);
			CONTINUE(nbBits);
		}
		if(FD_ISSET(s, &rfds)){
			nb = receiveDatas(&dataStructure, NULL, s);
			printf("Message reçu : '%i:%s'\n", dataStructure.id, dataStructure.query);
			CONTINUE(nbBits);
		}
	}
}

/**
	\fn question* setQuestions()
	\brief Définit un ensemble de questions puis les range dans un ordre aléatoire
		Une question est définie par l'intitulé de la question, les différentes
		options, et le numéro de la réponse (allant de 1 à 4)
	\return question* --> pointeur sur un ensemble de questions
*/
question* setQuestions() {
	static question questions[NB_QUESTIONS] = {
  		{
  			"Quelle partie de l'adresse réseau un routeur\nutilise-t-il pour transférer des paquets ? ",
  			{
  				"La partie hôte","L'adresse de diffusion","La partie réseau","L'adresse de passerelle"
  			},
  			3
  		},
  		{
  			"Quelle adresse permet une requête ping pour\nl'interface de bouclage ? ",
  			{
  				"126.0.0.1","127.0.0.0","0.0.0.0","127.0.0.1"
  			},
  			4
  		},
  		{
  			"Quelle valeur, qui se trouve dans un champ\n d'en-tête IPv4, est diminuée par chaque\n routeur qui reçoit un paquet ? ",
  			{
  				"Services différenciés","Décalage du fragment","Longueur d'en-tête","Time To Live"
  			},
  			4
  		},
  		{
  			"Quelle technique de migration du réseau encapsule\n les paquets IPv6 à l'intérieur des paquets IPv4\n pour les transporter à travers les \ninfrastructures réseau IPv4 ? ",
  			{
  				"Encapsulation","Traduction","Double pile","Tunneling"
  			},
  			4
  		},
  		{
  			"Quel type d'adresse IPv6 n'est pas routable et\n uniquement utilisé pour la communication \nsur un seul sous-réseau ? ",
  			{
  				"Adresse de monodiffusion globale","Adresse link-local","Adresse de bouclage","Adresse locale unique"
  			},
  			2
  		},
  		{
  			"Quelle est la notation de longueur du préfixe \npour le masque de sous-réseau 255.255.255.224 ? ",
  			{
  				"/25","/26","/27","/28"
  			},
  			3
  		},
  		{
  			"En dehors de DHCPv6, dans quel autre type \nd'adressage un routeur fournit-il dynamiquement\n des informations de configuration IPv6 aux hôtes ? ",
  			{
  				"ICMPv6","EUI-64","SLACK","SLAAC"
  			},
  			4
  		},
  		{
  			"Un type de périphérique se voit généralement attribuer\n une adresse IP statique. Lequel ? ",
  			{
  				"Clients","Serveurs Web","Concentrateurs","Imprimantes"
  			},
  			2
  		},
  		{
  			"Quelle méthode est utilisée pour gérer l'accès avec \ngestion des conflits sur un réseau sans fil ? ",
  			{
  				"CSMA/CD","Classement des priorités","CSMA/CA","Passage de jeton"
  			},
  			3
  		},
  		{
  			"Quelle est la méthode qui permet d'envoyer et de \nrecevoir simultanément des informations ? ",
  			{
  				"Bidirectionnel simultané","Double pile","Multiplexage","Simplex"
  			},
  			1
  		},
  		{
  			"Dans quelle couche du modèle OSI, une adresse \nphysique est-elle encapsulée ? ",
  			{
  				"Couche physique","Couche liaison de données","Couche réseau","Couche transport"
  			},
  			2
  		},
  		{
  			"Quelle couche du modèle de protocole TCP/IP \ndétermine le meilleur chemin sur le réseau ? ",
  			{
  				"Application","Transport","Internet","Accès réseau"
  			},
  			3
  		},
  		{
  			"Quel type de message est envoyé à un \ngroupe d'hôtes spécifique ? ",
  			{
  				"Monodiffusion","WhatsApp","Multidiffusion","Diffusion"
  			},
  			3
  		},
  		{
  			"Quelle adresse logique est utilisée pour la transmission\n des données à un réseau distant ? ",
  			{
  				"Adresse MAC de destination","Adresse IP de destination","Adresse MAC source","Adresse IP source"
  			},
  			2
  		},
  		{
  			"Quel est le terme générique utilisé pour décrire un \nbloc de données d'un modèle de réseau ? ",
  			{
  				"Trame","Paquet","Unité de données de protocole","Segment"
  			},
  			3
  		},
  		{
  			"Quelle est la principale fonction du protocole ARP ? ",
  			{
  				"Traduction des URL en adresses IP","Résolution d'adresses IPv4 en adresses MAC","Configuration IP dynamique","Lecture d'un PDF"
  			},
  			2
  		},
  		{
  			"Le nombre binaire 0000 1010, \nen hexadécimal, correspond à : ",
  			{
  				"0A","42","OC","0000 1010"
  			},
  			1
  		},
  		{
  			"Quel est l'équivalent décimal de la \nvaleur hexadécimale 3F ? ",
  			{
  				"18","59","62","63"
  			},
  			4
  		},
  		{
  			"Sur les réseaux Ethernet, l'adresse hexédécimale \nFF-FF-FF-FF-FF-FF représente l'adresse MAC : ",
  			{
  				"De monodiffusion","De diffusion","De multidiffusion","D'un serveur de Google"
  			},
  			2
  		},
  		{
  			"Quelle partie de la trame Ethernet permet \nde détecter les erreur ? ",
  			{
  				"La partie CRC","L'adresse MAC source","L'adresse MAC de destination","La réponse D"
  			},
  			1
  		}
	};

	srand(time(NULL));
	int j = 0;
	question temp;
	for(int i = 0 ; i < NB_QUESTIONS ; i++) {
		j = (rand() % NB_QUESTIONS);
		temp = questions[i];
		questions[i] = questions[j];
		questions[j] = temp;
	}

	return questions;
}

int waitEvent(fd_set *rfds, int s) {
	int nbBits;
	FD_ZERO(rfds);
	FD_SET(0, rfds);
	FD_SET(s, rfds);
	CHECK(nbBits = select(s + 1, rfds, NULL, NULL, NULL), "Erreur lors de l'appel à select");
	return nbBits;
}