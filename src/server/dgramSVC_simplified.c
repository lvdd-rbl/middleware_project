#include "../../libs/session.h"
#include "../../libs/window.h"

void getQuestions(question* questions){
	int i=0;
	FILE* file = NULL;
	if((file = fopen("./src/questions/questions", "r")) != NULL){
		while(fscanf(file, "%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n%[^\n]\n%i\n", questions[i].question, questions[i].options[0], questions[i].options[1], questions[i].options[2], questions[i].options[3], &questions[i].answer) != EOF){
			char * pch;
			while((pch = strstr(questions[i].question, "|")) != NULL){
				*pch = 10;
			}
			printf("%s\n%s | %s | %s | %s\nReponse : %i\n", questions[i].question, questions[i].options[0], questions[i].options[1], questions[i].options[2], questions[i].options[3], questions[i].answer);
			i++;
		}
	}
	srand(time(NULL));
	int j = 0;
	question temp;
	for(int i = 0 ; i < NB_QUESTIONS ; i++) {
		j = (rand() % NB_QUESTIONS);
		temp = questions[i];
		questions[i] = questions[j];
		questions[j] = temp;
	}
}

void queryHandler(socketInfo_t * sockInfo, int listenSocket) {
	int nb, fdMax = 0, retour;
	char pseudo[8];
	socklen_t cltLen;
	nbUsers = 0;
	fd_set fds,rfds;
	query dataStructure;
	cltLen = sizeof(sockInfo->clt);
	question questions[NB_QUESTIONS];
	printf("Nouveau lobby\n");
 	// Attente du choix et du pseudo du premier joueur
	receiveDatas(&dataStructure,NULL,sockInfo->s);
	strncpy(pseudo,dataStructure.query,8);
	pseudo[8]='\0';
	printf("%s\n", pseudo);
	// Si le joueur choisit "Créer une partie"
	if(dataStructure.id == 1) {
		printf("Joueur %s a crée une partie\n",pseudo);
		users[nbUsers].pseudo = strndup(pseudo,8);
	 	users[nbUsers].fd = sockInfo->s; 
		users[nbUsers].score = 0;	
		nbUsers++;
		
		// On accepte d'autres joueurs pendant LOGIN_WAIT secondes
		// L'alarme mettra le timer à 0 et débloquera la fonction accept
		// Qui renverra une erreur, d'où la nécessité du break
		// Note : sa_flags = 0 et non SA_RESTART
		alarm(LOGIN_WAIT);
  	printf("\n-- DEBUT LOBBY --\n");
  	serverMode = LOBBY_STATE;
  	timer = 1;
  	while(timer) {
	  		cltLen = sizeof(sockInfo->clt);
	  		nb = accept(listenSocket,
	  					(struct sockaddr *)&(sockInfo->clt), 
	  					&cltLen);
	  		if(nb<0) break;
	  		else {
		  		receiveDatas(&dataStructure,NULL,nb);

		  		// Enregistrement d'un nouveau joueur qui a rejoint la partie
		  		if(dataStructure.id == 2) {
			  		strncpy(pseudo,dataStructure.query,8);
			  		pseudo[8]='\0';
			  		printf("Nouveau joueur : %s\n",pseudo);
			  		users[nbUsers].pseudo = strndup(pseudo,8);
			  		users[nbUsers].fd = nb; 
			  		users[nbUsers].score = 0;

			  		// Envoi du pseudo du dernier joueur aux autres joueurs
			  		for(int i = 0 ; i < nbUsers ; i++) {
			  			writeDatas(&dataStructure,NULL,users[i].fd);
			  			usleep(300); // nécessaire entre chaque écriture
			  		}

			  		// Envoi des pseudos des autres joueurs au dernier joueur
			  		for(int i = 0 ; i < nbUsers ; i++) {
			  			dataStructure.query = users[i].pseudo;
			  			writeDatas(&dataStructure,NULL,users[nbUsers].fd);
			  			usleep(300);
			  		}

			  		nbUsers++;
		  		}
		 		}
	  	}	

	  	printf("\n-- FIN LOBBY --\nLes joueurs sont : \n");
	  	for(int i = 0 ; i < nbUsers ; i++) {
	  		printf("----> %s\n",users[i].pseudo);
	  		printf("--------->fd : %d\n",users[i].fd);
	  	}

	  	// Récupération des joueurs présents
	  	for(int i = 0 ; i < MAX_USERS ; i++) {
	  		if((nb = users[i].fd) != 0) {
	  			FD_SET(nb,&fds);
	  			// Récupération du plus grand file descriptor pour select()
	  			if(nb > fdMax)
	  				fdMax = nb;
	  		}
	  	}

	  	// Signale la fin du lobby à tous les joueurs présents avec le code 0
	  	dataStructure.id = 0;
	  	CHECK(select(fdMax+1,NULL,&fds,NULL,NULL),"Problem with select");
	  	for(int i = 0 ; i <= fdMax ; i++) { 
	  		if(FD_ISSET(i,&fds)) {

	  			writeDatas(&dataStructure,NULL,i);
	  		}
	  		printf("here\n");
	  	}
	  } 		// Si le joueur a choisi "Rejoindre une partie"
	else {
		printf("Joueur %s a tenté de rejoindre une partie : il n'y en a pas\n",pseudo);
		dataStructure.id == -2;
		writeDatas(&dataStructure,NULL,nb);
		exit(1);
	}
	printf("here\n");
	serverMode = GAME_STATE;
	getQuestions(questions);
	printf("\n-- Début du jeu --\n");
  sleep(2); 
	// size_t timeQ, timeA;

	// Envoi de chaque question avec une alarme de QUESTION_TIME secondes
	for(int i = 0 ; i < NB_QUESTIONS ; i++) {
		dataStructure.id = i;
		dataStructure.quest = questions[i];
		alarm(QUESTION_TIME);
	  	timer=1;
		for(int i = 0 ; i <= fdMax ; i++) { 
	  		if(FD_ISSET(i,&fds)) {
	  			// timeQ = time(NULL);
	  			// snprintf(dataStructure.query,sizeof dataStructure.query,"%zu",timeQ);
	  			printf("Envoi à %d : %i:%s:%s:%s:%s:%s:%s:%i\n",i, dataStructure.id, dataStructure.query, dataStructure.quest.question, dataStructure.quest.options[0], dataStructure.quest.options[1], dataStructure.quest.options[2], dataStructure.quest.options[3], dataStructure.quest.answer);

	  			writeDatas(&dataStructure,NULL,i);
	  		}
	  	}
	  	
	  	FD_ZERO(&rfds);
	  	rfds = fds;
	  	while(timer) {
	  		if(select(fdMax+1,&rfds,NULL,NULL,NULL) <= 0) 
	  			break;
	  		
	  		for(int i = 0 ; i <= fdMax ; i++) {
	  			if(FD_ISSET(i,&rfds)) {
	  				
	  				retour = receiveDatas(&dataStructure,NULL,i);
	  				// S'il n'y a plus de connexion
	  				if(retour == 0) {
	  					FD_CLR(i,&rfds);
	  					deleteFdUser(i);
	  					continue;
	  				}
	  				printf("Bonne réponse reçue de %d\n",i);

	  				// Incrémente le score du joueur correspondant
	  				for(int j = 0 ; j < MAX_USERS ; j++) 
	  					if(users[j].fd == i)
	  						users[j].score++;

	  				// for(int j = 0 ; j <= fdMax ; j++) {
	  				// 	if(FD_ISSET(j,&rfds) && j != i) {
	  				// 		// todo write que qqn a répondu correctement
	  				// 	}
	  				// }
	  				// sleep(2);
	  				// alarm(0);
	  				// timer = 0;
	  				// break;
	  			}
	  		}
	  	}
	  	printf("Timer terminé \n");
	}

	int meilleurJoueur = 0;
  	for(int j = 0 ; j < MAX_USERS ; j++) 
  		if(users[meilleurJoueur].score < users[j].score) 
  			meilleurJoueur = j;
  		
  	printf("\n-- Le gagnant est %s avec %d points --",users[meilleurJoueur].pseudo,
  														users[meilleurJoueur].score);

	// Signale la fin des questions et le gagnant à tous les joueurs
	dataStructure.id = -1;
	dataStructure.query = users[meilleurJoueur].pseudo;
  	for(int i = 0 ; i <= fdMax ; i++)  
  		if(FD_ISSET(i,&fds)) 
  			writeDatas(&dataStructure,NULL,i);
  	

  	


  	scanf("%s",NULL);
  	close(listenSocket);
	
}

void unitTests() {
	/**
		Test unitaire de computeAddress
	*/
	struct sockaddr_in sockAddr,sockAddr2;
	computeAddress(&sockAddr, "127.0.0.1:5000");
	printf("IP : %s, PORT : %d\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
	computeAddress(&sockAddr2, "168.52.145.87:4567");
	printf("IP : %s, PORT : %d\n", inet_ntoa(sockAddr2.sin_addr), ntohs(sockAddr2.sin_port));
	/**
	Test unitaire de socketInit	
	*/
	int s1, s2;
	socketInit(&s1, SOCK_DGRAM, NULL, 0);
	socketInit(&s2, SOCK_STREAM, "0.0.0.0:5000", 5);
	//scanf("%s", NULL);
	/**
	Test unitaire de getSockType
	*/
	printf("Type de s1 : %d | Type de s2 : %d | SOCK_DGRAM = %d | SOCK_STREAM = %d\n", getSockType(s1), getSockType(s2), SOCK_DGRAM, SOCK_STREAM);
}

/**
  \fn void deleteFdUser(int fd)
  \param fd --> file descriptor
  \brief Supprime un file descriptor de socket des données du tableau users
  \return void
*/
void deleteFdUser(int fd) {
	for(int i = 0 ; i < MAX_USERS ; i++) {
		if(users[i].fd == fd) {
			users[i].fd = 0;
			return;
		}
	}
}

int main(void) {
  	int listenSocket, nb, fdMax = 0, retour;
  	char pseudo[8];
  	socklen_t cltLen;
  	nbUsers = 0;
  	fd_set fds,rfds;
  	socketInfo_t sockInfo;
  	query dataStructure;
  	cltLen = sizeof(sockInfo.clt);

  	initSignaux();
  	socketInit(&listenSocket, SOCK_STREAM, "0.0.0.0:5000", 5);

  	printf("-- Lancement du serveur --\n");
  	FD_ZERO(&fds);
  	FD_ZERO(&rfds);
  	FD_SET(listenSocket,&fds);
		while(1){
			waitForClient(&sockInfo, listenSocket, queryHandler);

			while(!serverCanAccept) //Tant qu'on a pas recu de SIGUSR1 de la part du processus fils
				pause();
			printf("Je peux creer un nouveau lobby\n");
			serverCanAccept = 0;
		}
		



  	// while(1){
  	// 	waitForClient(&sockInfo[nbUsers], listenSocket, queryHandler);
  	// }
  return 0;
}



