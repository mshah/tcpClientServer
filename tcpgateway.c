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
#include <signal.h>
#define TRUE 1

/* This is the gateway program. It opens a socket,
and then begins an infinite loop. Each time through 
the loop it accepts a connection. Once it has accepted
a connection it will connect to the server:
Then it sends 25 packets with 1KB writes at a time
Then it closes the connection and waits for a new one.
The form of the commandline is: 'tcpserver' */

int main(int argc, char *argv[]){
	int sd, sda, length;
	struct sockaddr_in rcvr;
	static int buflen = 500;
	char buf[500];
	int rval;
	int seqno;
	struct timeval sndtime;
	struct timezone zone;
	
	signal(SIGPIPE, SIG_IGN);
	
	/* Get command line arguments */
	if (argc != 1){
		printf("Error: incorrect number of arguments\n");
		printf("Usage: tcpserver\n");
		exit(1);
	}
	
	/* create socket */
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0){
		perror("opening stream socket");
		exit(1);
	}
	
	/* name socket using wildcards */
	rcvr.sin_family = AF_INET;
	bzero((char*)&rcvr, sizeof(rcvr));
	rcvr.sin_addr.s_addr = INADDR_ANY;
	rcvr.sin_port = 0;
	if( bind(sd, (struct sockaddr*) &rcvr, sizeof(rcvr))){
		perror("binding socket name");
		exit(1);
	}
	
	/* find out assigned port number and print out */
	if (getsockname(sd, (struct sockaddr *)&rcvr, (socklen_t*)&length)){
		perror("getting socket name");
		exit(1);
	}
	
	if (gettimeofday(&sndtime, &zone) < 0){
		printf("Error: Client getting time\n");
	}
	printf("Gateway started on port #%d at time: %ld %ld\n", ntohs(rcvr.sin_port),
	sndtime.tv_sec, sndtime.tv_usec);

	/* Accept connections from the transmitter */
	listen(sd, 5);
	// wait until our client connects
	while( read( sda, buf, buflen) < 0){
		sda = accept(sd, 0, 0);
		if (sda == - 1){
			perror("accept");
		}
	}

	static int length500 = 500;	
	char serverName[length500];
	bzero(serverName, sizeof(serverName));
	char serverPort[length500];
	bzero(serverPort, sizeof(serverPort));
	char fileName[255];
	bzero(fileName, sizeof(fileName));
	int isConnected = 1;
	bcopy(buf, serverName, sizeof(serverName));
	bzero(buf, sizeof(buf));
	int total = 0;	// keep track of total packets
	
	if ((rval = read(sda, buf, buflen)) < 0){
		perror("Gateway error reading in filename \n");
	}
	bcopy(buf, fileName, sizeof(fileName));
	
	printf("Gateway accepting connection, filename= %s, server = %s/port= %s\n",
		fileName, serverName, serverPort);
	system("date");
	
	/* now connect to the server */
	int sock;
	struct sockaddr_in server, client;
	struct hostent *hp, *gethostbyname();
	
	// create a ring buffer t ostore packets
	int iCb = 0;
	char clientBuffer[1500];
	bzero(clientBuffer, sizeof(clientBuffer));
	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0){
		perror("opening stream socket");
		exit(1);
	}
	
	/* connect socket using name specified at command line */
	server.sin_family = AF_INET;
	hp = gethostbyname(serverName);
	if (hp == 0){
		printf("%s: unknown host \n", serverName);
		exit(1);
	}
	bcopy(hp->h_addr, &server.sin_addr, hp->h_length);
	server.sin_port =htons(atoi(serverPort));
	
	if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		perror("Error: Client connecting stream socket");
		exit(1);
	}
	
	/* find out assigned port number and print out */
	if( getsockname(sock, (struct sockaddr *)&client, (socklen_t*)&length)){
		perror("getting socket name");
		exit(1);
	}
	
	printf("Socket has port # %d\n");
	
	if(gettimeofday(&sndtime, &zone) < 0){
		perror("Error: client getting time");
	}
	
	printf("Gateway initiating connection to server start time: %ld %ld\n",
		sndtime.tv_sec, sndtime.tv_usec);
	system("date");
	bzero(buf, sizeof(buf));
	int length255 = 255;
	// write out the filename to the server
	if((rval = write(sock, fileName, length255)) < 0){
		perror("error writing fielName to server");
	}
	
	// while the server is sending me stuff
	while( read(sock, buf, buflen) > 0){
		// get the seq number from server
		char seq[1];
		seq[0] = buf[0];
		seqno = atoi(seq);
		
		// stick this in my buffer
		bcopy(buf, (char*) &clientBuffer[iCb * buflen], buflen);
		if(iCb == 2){
			iCb = 0;
		}else{
			iCb++;
		}
		
		// increment the seqno from server
		printf("Gateway received packet, seqno %d\n", seqno);
		// see if I can write this back to the mobile node
		int err = write(sda, buf, buflen);
		if (err < 0){
			printf("Gateway sees dropped connection \n");
			system("date");
			isConnected = 0;
			// listen(sd, 5);
			// wait until our client connects
			do{
				sda = accept(sd, 0, 0);
				if (sda == -1){
					perror("accept");
				}
				rval = read(sda, buf, buflen);
			}while (rval < 0);
			
			isConnected = 1;
			printf("Gateway continuing connection \n");
			system("date");
			
			// Get the last seq number the client had
			int clientSeq = 0;
			sscanf(buf, "%d", &clientSeq);
			printf("Gateway received sequence number %d from the client\n",
				clientSeq);
			system("date");
			
			// search our buffer for missing packets and send to client
			int i = 0;
			int iLow = 0;
			int minBufferedPacket = 0xFF;
			for (i = 0; i < 3; i++){
				iLow = iCb + i;
				if (iLow == 3){
					iLow = 0;
				}
				bcopy((char*) &clientBuffer[iLow * buflen], buf, buflen);
				int curSeq = 0;
				sscanf(buf, "%d", &curSeq);
				
				// keep track of the lowest in our buffer
				if(curSeq < minBufferedPacket){
					minBufferedPacket = curSeq;
				}
				
				if (curSeq > clientSeq){
					printf("Gateway resending packet %d \n", curSeq);
					system("date");
					
					if( write(sda, buf, buflen) < 0){
						perror("Gateway error sending buffered packets to client \n");
					}
					total++;
				}
			}
			
			if(clientSeq < minBufferedPacket){
				printf("Gateway cannot resend packets %d to %d\n", clientSeq,
					minBufferedPacket);
			}
		}else if(rval != buflen){
			printf("Sent packet of incorrect length %d\n", rval);
		}else{
			printf("Gateway sent packet, seqno = %d\n", seqno);
			total++;
		}
	}	// while read from server
	
	// disconnect from server
	close(sock);
	printf("Gateway complete\n");
	system("date");
	printf("Sent %d packets\n", total);
	
	// close connection to client
	close(sda);
	exit(0);
}
