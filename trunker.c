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
#define ARGN 3		//Arguments requiered by program
#define TIMEOUT 300	//300 seconds => 5 minutes.

char* NODE_IP;	//Node's clommunity IP(already in binary mode).

char* PSPROG="/var/local/cDistro/plug/resources/peerstreamer";

int insocket, outsocket;


void usage() {
	printf("UDPTrunker usage: \n");
}

void get_ip() {
	FILE * fp;
	char* line = NULL;
	size_t len = 0;

	system("/usr/sbin/avahi-ps info ip > /tmp/node_ip"); //Creates IP file

	fp = fopen("/tmp/node_ip", "r");
	if (fp == NULL)
	{
		perror("Could not open IP file, exiting now");
		exit(0);
	}

	if( getline(&line, &len, fp) < 0 )
	{
		perror("Could not read IP from file, exiting now");
		exit(0);
	}

	//If this point is reached, all open and read sentences were alright
	fclose(fp);
	NODE_IP = line;
	if(line) free(line);
	system("rm /tmp/node_ip");	//Deleting file
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

	//First of all, close sockets:
	close(insocket);
	close(outsocket);

	pid = fork();
	switch(pid) {
		case 0:
			//Child
			//must mutate to a script to kill the streamer instance(new pscontroller instance?)
		case -1:
			perror("Error while creating subprocess");
//			print_error("Error while creating subprocess");
		default:
			//Parent
			wait(&status);	//waiting for child process to end
			if (status < 0) 
			{
				printf("[ERROR] Error while killing peerstreamer");
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

int main(int argc, char* argv[]) {
	unsigned short inport, outport;
	
	//INITS
	init_signals(); //Initalizing signal handlers
	get_ip();		//Getting node IP
	if (argc == ARGN)
	{
		inport = atoi(argv[1]);
		outport = atoi(argv[2]);
	} else usage();


	//-------------Configuring in socket--------------------
	
	if ( (insocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0 )
	{
		perror("Failed to create (in)socket");
		return 0;
	}


	struct sockaddr_in address;
	address.sin_family = AF_INET;			//Setting as an external socket
	inet_pton(AF_INET, NODE_IP, address.sin_addr.s_addr);

//	address.sin_addr.s_addr = *NODE_IP;
	address.sin_port = htons( (unsigned short)inport );
	
	if ( bind( insocket, (const struct sockaddr*) &address, sizeof(struct sockaddr_in) ) < 0 )
	{
		perror("Failed to bind (in)socket");
		return 0;
	}
	//in socket is binded


	// -------------Configuring out socket--------------------

	if ( (outsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0 )
	{
		perror("Failed to create (out)socket");
		return 0;
	}

	//Once in socket is binded we can use the same structure to send.
	inet_pton(AF_INET, "127.0.0.1", address.sin_addr.s_addr);		//Not sure if can be done this way...
	address.sin_port = htons( (unsigned short) outport );


	//Receiving packets:

	while(1)
	{
		unsigned char packet_data[512];
		unsigned int max_packet_size = sizeof( packet_data);

		struct sockaddr_in from;
		socklen_t fromLength = sizeof( from);

		int bytes = recvfrom( insocket, (char*)packet_data, max_packet_size, 0, (struct sockaddr*)&from, &fromLength );
		// Packet received here, reseting alarm
		alarm(TIMEOUT);
		//Resending bytes:
		int sent_bytes = sendto( outsocket, (const char*)packet_data, bytes, 0, (const struct sockaddr*)&address, sizeof(struct sockaddr_in) );
		if(sent_bytes != bytes) perror("Failed to send packet"); //Not sure if should exit.. //Not sure if should exit....
		
		//Once data is received, we must re-send it to the internal port

	
		unsigned int from_address = ntohl( from.sin_addr.s_addr );
		unsigned int from_port = ntohs( from.sin_port );

	}

}
