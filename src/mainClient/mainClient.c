#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char* argv[]){
	printf("Appel système\n");
	char commandLine[200];
	strcat(commandLine, "xterm -fa 'Monospace' -fullscreen -e ./bin/client ");
	if(argc>1){
		strcat(commandLine, argv[1]);
	}
	printf("%s\n", commandLine);
	system(commandLine);
	return 0;
}
