#include "../../libs/session.h"

void queryHandler(socketInfo_t * sockInfo){
	pid_t pid = getpid();
	query dataStructure;
	printf("[PID = %d] Nouveau client\n", pid);
	do{
		receiveDatas(&dataStructure, NULL, sockInfo->s);
		printf("Message reçu : '%i:%s:%s'\n", dataStructure.id, dataStructure.query, dataStructure.info);  	
		switch(dataStructure.id){
			default :
				dataStructure.query = "OK, reçu";
				dataStructure.info = "ACK";
				printf("Envoi de : '%i:%s:%s' \n", dataStructure.id, dataStructure.query, dataStructure.info);
				writeDatas(&dataStructure, sockInfo, sockInfo->s);
				break;
		}
	}while(dataStructure.id != 0);
	printf("[PID = %d] Fermeture\n", pid);
}


void unitTests(){
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

int main(void){
	//unitTests();
  
  	int listenSocket;
  	socketInfo_t sockInfo;
  	finishChildren();
  	socketInit(&listenSocket, SOCK_STREAM, "0.0.0.0:5000", 5);
  	while(1){
  		waitForClient(&sockInfo, listenSocket, queryHandler);
  	}
  return 0;
}



