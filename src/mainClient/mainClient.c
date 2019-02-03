#include <stdlib.h>
#include <stdio.h>
int main(){
	printf("Appel système\n");
	system("xterm -fa 'Monospace' -fullscreen -e ./bin/client");
	return 0;
}
