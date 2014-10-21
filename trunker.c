#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>		// Requiered to execute shell commands from program
#include <unistd.h>		// Requiered to fork

/*
	@TODO LIST:
		- Set the alarm signal to TIMEOUT
		- Reset sigalarm every iteration
*/

// DEFINES
#define _ARGN 3		//Arguments requiered by program
#define TIMEOUT 300	//300 seconds => 5 minutes.


char* PSPROG="/var/local/cDistro/plug/resources/peerstreamer";

void usage() {
	printf("UDPTrunker usage: \n");
}

void print_error(char* error) {
	printf("[ERROR] %s\n", error);
}

/*
-- void timeout --
Kills peerstreamer instance and related, then exits
*/
void timeout() {
	// No packets arrived within TIMEOUT, must kill peerstreamer and call pscontroller
	//TODO: Must test this in a separeted program before procee
	pid_t pid;
	int status;
	pid = fork();
	switch(pid) {
		case 0:
			//Child
			//must mutate to a script to kill the streamer instance(new pscontroller instance?)
		case -1:
			perror("fork");
			print_error("Error while creating subprocess");
		default:
			//Parent
			wait(&status);	//waiting for child process to end
			if (status < 0) 
			{
				print_error("Error while killing peerstreamer");
				// We should try to kill some things here...
				exit(0);
			}
			system("/var/local/cDistro/plug/resources/peerstreamer info"); // Kills remaining processes related with PS instance.
			exit(0);
	}
}

void alarm_handler(int sig) {
	signal(SIGALRM, SIG_IGN);		//Ignoring signal; things to do before exiting
	timeout();
}

void init_signals(){
	signal(SIGALRM, alarm_handler);
}

int main(int argc, (char *) argv[]) {
	unsigned short port = atoi(argv[1]);
	int handle;

	init_signals();

	if (argc != _ARGN) usage();

	if ( (handle = socket( AF_INET, SOCK_DGRAM, IPROTO_UDP) ) < 0 )
	{
		perror("Socket");
		print_error("failed to create socket" );
		return false;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( (unsigned short) port );

	if ( bind( handle, (const sockaddr*) &address, sizeof(sockaddr_in) ) < 0 )
	{
		perror("Binding");
		print_error("Failed to bind socket");
		return false;
	}

	//Not setting the socket to a non-blocking, it's useless to our purpose...
	
	//Receiving packets:

	while(true)
	{
		unsigned char packet_data[512];
		unsigned int max_packet_size = sizeof( packet_data);

		sockaddr_in from;
		socklen_t fromLength = sizeof( from);

		int bytes = recvfrom( socket, (char*)packet_data, max_packet_size, 0, (sockaddr*)&from, &fromLength );

		//if (bytes <= 0) break;

		unsigned int from_address = ntohl( from.sin_addr.s_addr );
		unsigned int from_port = ntohs( from.sin_port );

		//Packet received

	}

}
