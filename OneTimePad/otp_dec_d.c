/*****************************************************************************************
 * Author: 	Keisha Arnold
 * Date: 	03/17/2017 CS344-400
 * Program 2: 	otp_dec_d.c 
 * Description:	This program will run in the background as a daemon. It's function is to 
 * 		perform decryption using a one-time pad-like system.  It listens on a 
 * 		particular port/socket and when a connection is made another socket is
 * 		created for communication.  It can support up to 5 concurrent socket
 * 		connections. After making sure it is communicating with otp_dec it receives 
 * 		ciphertext and a key via the same communication socket. After decryption the 
 * 		plaintext is sent back to otp_dec.  
 * 		Citing help from server.c file provided by the instructor. 
 ******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

#include "otp.h"


int main(int argc, char *argv[]) {
    int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
    socklen_t sizeOfClientInfo;		// size of client address
    char buffer[256];
    memset(buffer, '\0', sizeof(buffer));
    struct sockaddr_in serverAddress, clientAddress;

    pid_t spawnpid;
    int status;

    int ciphertextLength;
    int keyLength;

    char client[] = "otp_dec";

    //validate number of arguments
    if (argc < 2) { 
	fprintf(stderr,"USAGE: %s port\n", argv[0]); 
	exit(1); 
    }

    // Set up the address struct for this process (the server/daemon)
    memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

    // Set up the socket
    listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (listenSocketFD < 0) {
	error("ERROR opening socket\n");
    }

    // Enable the socket to begin listening
    if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {  // Connect socket to port
	error("ERROR on binding\n");
    }
    // The server will ignore any connection attempts until you tell the socket to start listening
    listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

    while(1) { // Infinite loop continually processes connections from clients 
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0) {
	    error("ERROR on accept\n");
        }

   	spawnpid = fork();
        switch(spawnpid) {
	    case -1: {  //error!
		close(establishedConnectionFD); // Close the existing socket which is connected to the client
		error("ERROR forking\n");
		break;
	    }
    	    case 0: { // the child is the process that's currently running	
		if (!checkClient(buffer, establishedConnectionFD, client)) {
		    // Send mismatch message, close fd    		    
		    charsRead = send(establishedConnectionFD, "client mismatch", 15, 0); 
        	    if (charsRead < 0) {
	    	        error("ERROR writing to socket\n");
        	    }
		    busySending(establishedConnectionFD);
		    //client needs to close fd on their end if rejected, then client will exit(1)
		    // Don't exit! Just break so we can keep accepting connections
		    break; 
		}
		// Client is otp_dec, send approval message
		else {	
		    charsRead = send(establishedConnectionFD, "client match", 12, 0); 
        	    if (charsRead < 0) {
	    	        error("ERROR writing to socket\n");
        	    }
		} 
		 busySending(establishedConnectionFD);
		
		// Receive length of ciphertext and key, so we know how much data to expect
        	charsRead = recv(establishedConnectionFD, &ciphertextLength, sizeof(ciphertextLength), 0);
	    	if (charsRead < 0) {
		    error("ERROR reading from socket\n");
		}
		//printf("SERVER: Ciphertext Length: %d\n", ciphertextLength);
		//fflush(stdout);

        	charsRead = recv(establishedConnectionFD, &keyLength, sizeof(keyLength), 0);
        	if (charsRead < 0) {
	    	    error("ERROR reading from socket\n");
		}
		//printf("SERVER: Key Length: %d\n", keyLength);
		//fflush(stdout);

		// Receive ciphertext and key from otp_dec
		char ciphertext[ciphertextLength];
		char key[keyLength];
		memset(ciphertext, '\0', sizeof(ciphertext));
		memset(key, '\0', sizeof(key));

        	charsRead = recv(establishedConnectionFD, &ciphertext, sizeof(ciphertext), MSG_WAITALL);
	    	if (charsRead < 0) {
		    error("ERROR reading from socket\n");
		}
		//printf("SERVER: Plaintext from the client: \"%s\"\n", plaintext);
                //fflush(stdout);

        	charsRead = recv(establishedConnectionFD, &key, sizeof(key), MSG_WAITALL);
        	if (charsRead < 0) {
	    	    error("ERROR reading from socket\n");
		}
		//printf("SERVER: Key from the client: \"%s\"\n", key);
                //fflush(stdout);

		// Decrypt the ciphertext
		char plaintext[ciphertextLength];
		memset(plaintext, '\0', sizeof(plaintext));
		//doDecryption(plaintext, ciphertext, ciphertextLength, key);
		doDecryption(plaintext, ciphertext, key);
		
		
		// Send plaintext to otp_dec
		charsRead = send(establishedConnectionFD, plaintext, sizeof(plaintext), 0); 
        	if (charsRead < 0) {
	    	    error("ERROR writing to socket\n");
        	}
		 busySending(establishedConnectionFD);
		// Close socket
		close(establishedConnectionFD);
		exit(0);
		break;

	    }
	    default: {  // The parent is the process that's currently running
        	close(establishedConnectionFD); // Close the existing socket which is connected to the client
		//wait for state changes in child processes 
		wait(NULL);
	    }
	
    	} // End of switch statement
        //close(establishedConnectionFD); // Close the existing socket which is connected to the client
    } // End of while loop
    close(listenSocketFD); // Close the listening socket

    return 0;
}



