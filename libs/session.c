#include "session.h"
/**
  \file session.c
  \author Samuel Sueur
  \version v1.0
  \brief Librairie contenant des fonctions permettant de gérer les actions entre clients et serveur.
  \date 6 décembre 2018
*/


/**
  \fn assignResources(socketInfo_t * sockInfo, int listenSocket, void (*queryHandler)(socketInfo_t *))
  \param sockInfo --> la structure contenant des informations sur la socket qui sera utilisé
  \param listenSocket --> La socket d'écoute utilisée.
  \param queryHandler --> La fonction appelée pour traiter les requêtes clients
  \brief Fonction permettant d'assigner un processus fils à un client. Le processus fils exécutera la fonction passée en paramètre.
  \return void
*/
void assignResources(socketInfo_t * sockInfo, int listenSocket, void (*queryHandler)(socketInfo_t *, int)){
  pid_t pid;
  CHECK(pid = fork(), "Impossible d'assigner un processus au nouveau client");
  if(pid == 0){
    queryHandler(sockInfo, listenSocket);
    CHECK(close(sockInfo->s), "Impossible de fermer la socket de dialogue");
    exit(EXIT_SUCCESS);
  }
}

/**
	\fn void bindAddress(int socket, struct sockaddr_in * sockAddr)
	\param socket --> le numéro de la socket
	\param sockAddr --> la structure à utiliser pour bind l'adresse
	\brief Fonction enveloppe pour l'appel système bind()
	\return void
*/
void bindAddress(int *socket, struct sockaddr_in sockAddr){
	CHECK(bind(*socket, (struct sockaddr *) &sockAddr, sizeof(sockAddr)), "Impossible de bind la socket d'écoute");
}

/**
  \fn void computeAddress(struct sockaddr_in * addr, char * address)
  \param addr --> la structure à remplir
  \param address --> la chaîne de caractère à décomposer "w.x.y.z:p"
  \brief Fonction permettant de remplir la structure sockaddr_in en paramètre
  \return void
*/
void computeAddress(struct sockaddr_in * addr, char * address){
	if(address != NULL){
		int port;
		char ipAddress[20];
		// printf("%s\n", address);
		sscanf(address, "%[^:]:%i", ipAddress, &port);
		// printf("SOCKET : [PORT = %i] | [IP = %s]\n",port, ipAddress);
		addr->sin_family = PF_INET;
		addr->sin_port = htons(port);
		addr->sin_addr.s_addr = inet_addr(ipAddress);
		memset(&(addr->sin_zero), 0, 8);
	}
}

/**
  \fn void connectToServer(int s, char * address)
  \param s --> La socket à utiliser.
  \param address --> la chaîne de caractère à décomposer "w.x.y.z:p"
  \brief Fonction permettant de se connecter au serveur. Aucun effet si on est sur une connexion de type DGRAM.
  \return void
*/
void connectToServer(int s, char * address){
  switch(getSockType(s)){
    case SOCK_STREAM :{
		struct sockaddr_in sockAddr;
		computeAddress(&sockAddr, address);
		// printf("Connecté à %s, port : %d\n", inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
		CHECK(connect(s, (struct sockaddr *) &sockAddr, sizeof(sockAddr)), "Impossible pour le client de se connecter au serveur");
		break;
	}
    default :
      printf("TYPE : %d | TYPE ATTENDU : %d | connectToServer() n'a pas d'utilité pour le type de socket utilisé\n", getSockType(s), SOCK_STREAM);
      break;
  }
}

/**
	\fn void deroute(int numSignal)
	\param numSignal --> le numéro du signal à traiter
	\brief Fonction de traitement des signaux bloqués
	\return void
*/
void deroute(int numSignal){
  pid_t pidFilsFini;
  int status;
  switch(numSignal){
    case SIGCHLD: CHECK(pidFilsFini = wait(&status), "Problem while waiting");
      if(WIFEXITED(status))
        printf("Fin du fils [PID=%i] --> Exit avec [STATUS=%i]\n", getpid(), status);
      if(WTERMSIG(status))
        printf("Fin du fils [PID=%i] --> SIGNAL avec [STATUS=%i]\n", getpid(), status);
      break;
      
    case SIGUSR1:
    	serverCanAccept = 1;
    	break;

    case SIGALRM : 
    	if(serverMode == LOBBY_STATE){
    		kill(getppid(), SIGUSR1); //Si on est dans le lobby on signal au père qu'il peut créer une nouvelle partie
    	}
      timer = 0;
      timerAnswer = 0;
      break;

    case SIGWINCH : 

      break;
  }
}

/**
	\fn void initSignaux(void)
	\brief Fonction permettant au processus en question de pouvoir bloquer
    et traiter les différents signaux
	\return void
*/
void initSignaux(void){
   struct sigaction newAct;
   newAct.sa_handler = deroute;
   newAct.sa_flags = 0;
   CHECK(sigemptyset(&newAct.sa_mask), "Erreur lors du nettoyage du mask");
   CHECK(sigaction(SIGUSR1, &newAct, NULL), "Erreur lors de la mise en place du déroutement de signal");
   CHECK(sigaction(SIGCHLD, &newAct, NULL), "Erreur lors de la mise en place du déroutement de signal");
   CHECK(sigaction(SIGALRM, &newAct, NULL), "Erreur lors de la mise en place du déroutement de signal");
   CHECK(sigaction(SIGWINCH, &newAct, NULL), "Erreur lors de la mise en place du déroutement de signal");
}

/**
	\fn int getSockAddr(int s, struct sockaddr_in * sockAddr)
	\param s --> la socket dont on souhaite obtenir l'adresse
	\param sockAddr --> la structure à remplir
	\brief Fonction permettant d'obtenir l'adresse d'une socket
	\return int : le type de la socket
*/
void getSockAddr(int s, struct sockaddr_in * sockAddr){
	int size = sizeof(*sockAddr);
	CHECK(getsockname(s, (struct sockaddr *) sockAddr, &size), "Impossible de récupérer les informations");
}

/**
	\fn int getSockType(int s)
	\param s --> la socket dont on souhaite obtenir le type
	\brief Fonction permettant d'obtenir le type d'une socket
	\return int : le type de la socket
*/
int getSockType(int s){
	int type, length = sizeof(int);
	CHECK(getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &length), "Impossible de récupérer le type de la socket");
	return type;
}

/**
	\fn void listenMode(int s, int queue)
	\param s --> la socket
	\param queue --> la capacité maximale
	\brief Fonction enveloppe pour l'appel système listen()
	\return void
*/
void listenMode(int s, int queue){
	 CHECK(listen(s, queue), "Erreur lors de la mise en écoute de la socket");
}

/**
	\fn void receiveDatas(query * query, socketInfo_t * sockInfo, int stream)
	\param query --> La structure pour récupérer les données
	\param sockInfo --> La structure contenant les informations sur le client. NULL si on est en SOCK_STREAM
	\param stream --> Le canal dont on souhaite obtenir les données
	\brief Fonction permettant de recevoir des données.
	\return void
*/
int receiveDatas(query * query, socketInfo_t * sockInfo, int stream){
  int cltLen = sizeof(sockInfo->clt);
  int type;
  int nb = 0;
  if(sockInfo == NULL){ 
  	type = SOCK_STREAM;
  } else {
  	type = getSockType(sockInfo->s);
  } 
  switch(type){
    case SOCK_DGRAM :
      CHECK(recvfrom(stream, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &(sockInfo->clt), &cltLen), "Impossible de lire les données");
      break;
    case SOCK_STREAM :
        CHECK(nb = read(stream, buffer, sizeof(buffer) - 1), "Impossible de lire les données");
      break;
  }
  *query = stringToQuery(buffer);
  return nb;
}

/**
  \fn void socketInit(int *s, int type, char * address, int flag)
  \param s --> La socket à initialiser
  \param type --> Le type de la socket
  \param address --> L'adresse à utiliser pour la socket
  \param flag --> Si supérieur à 0, mise à l'écoute avec la taille spécifiée
  \brief Fonction permettant d'initialiser une socket
  \return void
*/
void socketInit(int *s, int type, char * address, int flag){
  CHECK(*s = socket(AF_INET, type, 0), "Impossible de créer une socket");

  // Temp code /////////
  struct linger {
                      int l_onoff;    /* linger active */
                      int l_linger;   /* how many seconds to linger for */
                  };
  struct linger lin;
  lin.l_onoff = 0;
  lin.l_linger = 0;
  setsockopt(*s, SOL_SOCKET, SO_LINGER, (const char *)&lin, sizeof(int));
  if (setsockopt(*s, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
    printf("setsockopt(SO_REUSEADDR) failed\n");
  // Temp code ///////////

  if(address != NULL){
  	struct sockaddr_in sockAddr;
  	computeAddress(&sockAddr, address);
  	bindAddress(s, sockAddr);
	if(flag > 0 && type == SOCK_STREAM){
  		listenMode(*s, flag);
  		// printf("Mise sur écoute\n");
  	}
  } else {
		// printf("Pas de bind d'adresse\n");
  }
}

/**
  void waitForClient(socketInfo_t * sockInfo, int listenSocket, void (*f)(socketInfo_t *))
  \param sockInfo --> la structure à remplir avec les informations du client
  \param listenSocket --> La socket d'écoute
  \param f --> La fonction à appeler lorsque le client a été accepté.
  \brief Fonction permettant d'accepter un client, de lui assigner un processus fils, et d'appeler une fonction pour traiter ses demandes. Inutilisable si la connexion est de type DGRAM.
  \return void
*/
void waitForClient(socketInfo_t * sockInfo, int listenSocket, void (*f)(socketInfo_t *, int)){
  int cltLen = sizeof(sockInfo->clt);
  int sockNum;
  CHECK(sockInfo->s = accept(listenSocket, (struct sockaddr *) &(sockInfo->clt), &cltLen), "Erreur lors de la tentative de connexion");
  assignResources(sockInfo, listenSocket, f);
  CHECK(close(sockInfo->s), "Impossible de fermer la socket de dialogue");
}

/**
	\fn void writeDatas(query * query, socketInfo_t * sockInfo, int s)
	\param query --> la structure accueillant les données
	\param sockInfo --> la structure contenant des informations sur la socket (uniquement pour du DGRAM !!)
	\param s --> la socket utilisée
	\brief Fonction permettant d'envoyer des données.
	\return void
*/
void writeDatas(query * query, socketInfo_t * sockInfo, int s){
  char * answer = queryToString(query);
  switch(getSockType(s)){
    case SOCK_DGRAM :
      CHECK(sendto(s, answer, strlen(answer) + 1, 0, (struct sockaddr *) &(sockInfo->clt), sizeof(sockInfo->clt)), "Echec lors de l'envoi de la réponse");
      break;
    case SOCK_STREAM :
      CHECK(write(s, answer, strlen(answer) + 1), "Can't send");
      break;      
  }
}


