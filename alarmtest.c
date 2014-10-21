#include <stdio.h>
#include <signal.h>

void alhandler(int sig) {
	signal(SIGALRM, SIG_IGN);
	printf("Caca\n");

	signal(SIGALRM, alhandler);
}

int main() {
	signal(SIGALRM, alhandler);
	
	alarm(2);
	sleep(1);
	alarm(5);

	while(1);
}
