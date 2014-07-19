/* TCPClient.c */
#include <unistd.h>
#include <strings.h>
#include <sys/types.h> /* for open */
#include <sys/socket.h>
#include <netinet/in.h> /* for gethostbyname */
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h> /* for gettimeofday */
#include <stdlib.h> /* for atoi */

#define TRUE 1

/* This is the server program. It opens a socket,
and then beings an infinite loop. Each time through
the loop it accepts a connection. Then it sends 25 packets with 1KB writes at a time.
Then it closes the connection and waits for a new one.
The form of the comand line is: 'tcpserver' */

int main(int argc, char *argv[]){
	int sd, sda, length, seqno, rval;
	struct sockaddr_in rcvr;
	static int buflen = 500;
	car buf[buflen];
	struct timeval sndtime;
	struct timezone zone;
	
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
	if( bind(sd, struct sockaddr*) &rcvr, sizeof(rcvr))){
		perror("binding socket name");
		exit(1);
	}
	
	/* find out assigned port number and print out */
	if (getsockname(sd, (struct sockaddr *)&rcvr, (socklen_t*)&length)){
		perror("getting socket name");
		exit(1);
	}
	printf("Server started on port # %d, at time %ld %ld\n", ntohs(rcvr.sin_port),
		sndtime.tv_sec, sndtime.tv_usec);
		
	// file name
	char file[255];
	int fileLen = 255;
	bzero(file, sizeof(file));
	
	/* Accept connections from the transmitter */
	listen(sd, 5);
	do{
		sda = accept(sd, 0, 0);
		if (sda == -1){
			perror("accept");
		}else{
			if(gettimeofday(&sndtime, &zone) < 0){
				perror("Getting time");
			}
			
			printf("Server accepted connection at time: %ld %ld\n", sndtime.tv_sec,
				sndtime.tv_usec);
			bzero(buf, sizeof(buf));
			
			// get the filename
			if ((rval = read(sda, file, fileLen)) < 0){
				perror("server error reading filename \n");
			}
			
			printf("client requested file %s\n", file);
			
			// open this file up and read in 500 byte chunks
			FILE *pFile = fopen(file, "r");
			if (pFile == NULL){
				perror("error opening requested file \n");
			}
			int filebuflen = 498;
			char* filebuf;
			filebuf = (char*) malloc(sizeof(char)* filebuflen);
			fseek(pFile, 0, SEEK_SET);
			long fileSize = ftel(pFile);
			int numPackets = fileSize/498;
			seqno= 1;
			while(chunksRead < numPackets){
				/* create a packet */
				chunksRead++;
				fread(filebuf, 1, filebuflen, pFile);
				fseek(pFile, filebuflen * chunksRead, SEEK_SET);
				sprintf(buf, "%d%s", chunksRead, filebuf);
				printf("Packet after read: %s\n", buf);
				
				if((rval = write(sda, buf, buflen)) < 0){
					perror("writing on stream socket");
				}else if (rval != buflen){
					printf("Sent packet of incorrect length %d\n", rval);
				}
				printf("Server sent packet %d, size: %d\n", seqno, rval);
				seqno++;
				sleep(1);	
			} // while, all done sending full 500 byte packets
			
			free(filebuf);
			
			// last chunk left
			fseek(pFile, 0, SEEK_CUR);
			long filePos = ftell(pFile);
			int byteLeft = fileSize - filePos;
			
			filebuf = (char*) malloc( sizeof(char) * byteLeft);
			printf("bytesLeft: %d\n", byteLeft);
			bzero(filebuf, sizeof(filebuf));
			fread(filebuf, 1, byteLeft, pFile);
			seqno++;
			bzero(buf, sizeof(buf));
			sprintf(buf, "%d%s", seqno, filebuf);
			if((rval = write(sda, buf, byteLeft)) < 0){
				perror("writing on stream socket");
			}
			printf("Server sent packet %d, size: %d \n", seqno, rval);
			free(filebuf;
			
			fclose(pFile);
			printf("Server end time \n");
			system("date");
			printf("Sent %d packets \n", seqno -1);
		}
		close(sda);
	}while(TRUE);
}