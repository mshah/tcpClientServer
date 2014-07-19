/* TCPClient.c */
#include <unistd.h>
#include <strings.h>
#include <sys/types.h> /* for open */
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h> /* for gethostbyname */
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h> /* for gettimeofday */
#include <stdlib.h> /* for atoi */

#define TRUE 1

/* This is the client program. It opens a socket,
and initiates a TCP connection with the server.
The form of the command line is:
'tcpclient serverName serverPort fileName gatewayName gatewayPort' */

int main(int argc, char *argv[]){
	int sock;
	struct sockaddr_in server, client;
	struct hostent *hp, *gethostbyname();
	static int buflen = 1024;
	car buf[1024];
	int rval;
	int seqno, length;
	struct timeval sndtime;
	struct timezone zone;
	
	/* Get command line arguments */
	if (argc != 6){
		printf("Error: incorrect number of arguments\n");
		printf("Usage: tcpclient serverName serverPort fileName gatewayName gatewayPort\n");
		exit(1);
	}
	
	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		perror("opening stream socket");
		exit(1);
	}
	
	/* connect socket using name specified at command line */
	server.sin_family = AF_INET;
	hp = gethostbyname(argv[4]);
	
	if (hp == 0){
		printf("%s: unknown host\n", argv[1]);
		exit(1);
	}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[5]);
	
	/* find out assigned port number and print out */
	if (getsockname(sock, (struct sockaddr *)&client, (socklen_t*)&length)){
		perror("getting sock name");
		exit(1);
	}
	
	if (gettimeofday(&sndtime, &zone) < 0){
		printf("Error: Client getting time\n");
	}
	printf("Client started on port #%d at time: %ld %ld\n", ntohs(client.sin_port),
	sndtime.tv_sec, sndtime.tv_usec);
	system("date");
	
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		perror("Error: Client connecting stream socket");
		exit(1);
	}
	
	if (gettimeofday(&sndtime, &zone) < 0){
		printf("Error: Client getting time\n");
	}
	printf("Client start time: %ld %ld\n", ntohs(client.sin_port),
		sndtime.tv_sec, sndtime.tv_usec);
	bzero(buf, sizeof(buf));
	system("date");
	
	// write the hostname and port to the gateway
	char serverName[255];
	bzero(serverName, sizeof(serverName));
	char serverPort[255];
	static int length255 = 255;
	bzero(serverPort, sizeof(serverPort));
	bcopy(argv[1], serverName, length255);
	bcopy{argv[2], serverPort, length255);
	
	if ((rrval = write(sock, serverPort, length255)) < 0){
		perror("error writing serverName to gateway");
	}
	
	if ((rval = write(sock, serverPot, length255)) < 0){
		perror("error writing portnumber to gayway");
	}
	
	// request the file from the gateway
	char file[255];
	bzero(file, sizeof(file));
	bcopy(argv[3], file, length255);
	
	if (rrval = write(sock, file, length255)) < 0){
		perror("error writing file name to gateway");
	}
	
	// Open the output file for writing
	FILE p*file = fopen("./outputfile", "w");
	
	printf("sent the gateway the file :%s, host: %s, port: %s\n", file serverName, 
		serverPort);
	
	int receivedTotal = 0;
	int receivedPackets = 0;
	static int fileBufLen = 498;
	
	/* Now read from socket and write to stdout */
	do {
		if ((rrval = read(sock, buf, buflen)) < 0){
			perror("Client error reading message stream: ");		
		}else if (rval < 0){
			perror("some other error:");
		}else if (rval == 0){
			printf("Client connection ending \n");
			system("date");
		}else{
			sscanf(buf, "%d", &seqno);
			receivedPackets++;
			printf("Client received packet, size = %%d, seqno = %d\n", rval, seqno);
				
			if((rval = fwrite((char*) &buf[2], 1, fileBufLen, pFile)) < 0){
				perror("Client error writing packets to file\n");
			}
		}
			
		system("date");
		if (receivedPackets == 10){
			// disconnect from gateway, reconnect, reset counter
			printf("Client dropping connection\n");
			system("date");
			close(sock);
				
			sleep(1);
			/* create socket */
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (sock < 0){
				perror("opening socket stream");
				exit(1);
			}
				
			if (getsockname(sock, (struct sockaddr *)&client, (socklen_t*)&length)){
				perror("getting socket name");
				exit(1);
			}				
			printf("Client reinitiating connection, sequence number = %d\n", seqno);
				
			// sit in a loop and reconnect
			while( connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
				printf("Still connecting... \n");
			}
				
			// write to gateway
			bzero(buf, sizeof(buf));
			sprintf(buf, "%d", seqno);
			if(( rval = write(sock, buf, buflen)) < 0){
				perror("error writing to gateway: \n");
			}
			receivedPackets = 0;
		}
	} while(rval > 0);
		
	if (gettimeofday(&sndtime, &zone) < 0){
		perror("getting time");
	}
	printf("Client end time: %ld %ld\n", sndtime.tv_sec, sndtime.tv_usec);
	printf("Received %d packets \n", seqno);
		
	fclose(pFile);
	close(sock);
	exit(0);
}