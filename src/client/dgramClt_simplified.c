#include "../../libs/session.h"



int waitEvent(fd_set *rfds, int s){
	int nbBits;
	FD_ZERO(rfds);
	FD_SET(0, rfds);
	FD_SET(s, rfds);
	CHECK(nbBits = select(s + 1, rfds, NULL, NULL, NULL), "Erreur lors de l'appel à select");
	return nbBits;
}


void serverDialogue(int s){
	int nb = 1, nbBits;
	fd_set rfds;
	query dataStructure;
	while(nb != 0){
		nbBits = waitEvent(&rfds, s);
		if(FD_ISSET(0, &rfds)){
			receiveDatas(&dataStructure, NULL, 0);
			printf("Envoi de : '%i:%s:%s'\n", dataStructure.id, dataStructure.query, dataStructure.info);
			writeDatas(&dataStructure, NULL, s);
			CONTINUE(nbBits);
		}
		if(FD_ISSET(s, &rfds)){
			nb = receiveDatas(&dataStructure, NULL, s);
			printf("Message reçu : '%i:%s:%s'\n", dataStructure.id, dataStructure.query, dataStructure.info);
		}
			CONTINUE(nbBits);
	}
}



int main(int argc, char * argv[]){
	int s;
	socketInit(&s, SOCK_STREAM, NULL, 0);
	if(argc>1)
		connectToServer(s, argv[1]);
	else
		connectToServer(s, "127.0.0.1:5000");
	serverDialogue(s);
	return 0;
}
