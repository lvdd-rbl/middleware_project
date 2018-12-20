#include "serialize.h" 
/**
  \file serialize.c
  \author Lucas Van Den Driessche
  \version v1.0
  \brief Librairie contenant des fonctions permettant la conversion 
  	des structures en chaînes de caractère et inversement.
  \date 9 décembre 2018
*/

/**
  \fn char * queryToString(query * q)
  \param q --> La structure de la requête à sérialiser
  \brief Fonction permettant de transformer la structure query en chaine de caractères
  \return char *
*/
char * queryToString(query * q) {
	char * buffer = malloc(sizeof(q->id) + strlen(q->query) + strlen(q->info));
	sprintf(buffer,"%i:%s:%s",q->id,q->query,q->info);
	return buffer;
}

/**
  \fn query stringToQuery(char * s)
  \param s -> chaîne de caractères contenant les informations de la requête
  \brief Fonction permettant de transformer la chaine de caractères en structure query
  \return query
*/
query stringToQuery(char * s) {
	query query2;
  query2.query = malloc(BUFFER_SIZE);
  query2.info = malloc(BUFFER_SIZE);
  sscanf(s,"%i:%[^:]:%[^\n]",&(query2.id),query2.query,query2.info);
	return query2;
}






































  // int stringStart = 0;
 //  int stringStart2 = 0;
 //  char * buf = malloc(50*sizeof(char));
 //  char * buf2 = malloc(50*sizeof(char));
 //  char * buf3 = malloc(50*sizeof(char));
 //  char * buf4 = malloc(50*sizeof(char));
 //  char * buf5 = malloc(50*sizeof(char));
 //  printf("ENVOYE A STRINGTOQUERY : %s\n",s);
  // sscanf(s,"%i:%n:%s%n\n",&(query2.id),&stringStart,buf,&stringStart2);
 //  printf("STRING 1 : %i\n",stringStart);
 //  printf("STRING 2 : %i\n",stringStart2);
 //  extract(s,buf2,buf3,stringStart,stringStart2-stringStart+1);
 //  extract(s,buf4,buf5,stringStart2,stringStart2-stringStart+1);
 //  printf("BUF 3 : %s\n",buf3);
 //  query2.query = malloc(strlen(s+stringStart));
 //  strcpy(query2.query,buf3);

 //  query2.info = malloc(strlen(s+stringStart2));
 //  strcpy(query2.info,s+stringStart2);
  // strcpy(query2.query,substring(s,stringStart,stringStart2));
  // query2.info = malloc(strlen(substring(s+stringStart2)));
  // strcpy(query2.info,s+stringStart2);
  // query2.query = malloc(strlen(s+stringStart+1));
 //  query2.info = malloc(strlen(s+stringStart2+1)); 
  // strcpy(query2.query,s+stringStart+"\0");
 // //  strcpy(query2.info,s+stringStart2+"\0");
 //  query2.query = malloc(strlen(s));
 //  query2.info = malloc(strlen(s));
