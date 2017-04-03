/*****************************************************************************************
 * Author: 	Keisha Arnold
 * Date: 	03/17/2017 CS344-400
 * Program 4: 	otp_enc.c 
 * Description: This program connects to otp_enc_d and asks it to perform a one-time pad
 * 		style encryption. It validates if the plaintext or key files contain bad
 * 		characters and verifies that the key is at least as long as the plaintext.
 * 		otp_enc should not be able to connect to otp_dec_d. If this happens, 
 * 		otp_enc will report the rejection, then terminate. When otp_enc receives
 * 		the ciphertext back from otp_enc_d it will output it to stdout. 
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h> 
#include <sys/ioctl.h>

#include "otp.h"


int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
    
 	int file_descriptor;
	int plaintextLength;
	int keyLength;
	int ptCharsRead;
	int keyCharsRead;

	char id[] = "otp_enc";

	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args

	// Validate the files first before setting up the socket
	// Open plaintext file for reading
	file_descriptor = open(argv[1], O_RDONLY);
	if(file_descriptor == -1) {
	    fprintf(stderr, "ERROR: cannot open %s\n", argv[1]);
	    exit(1);
	}

	// Go to the end of the file
	// lseek returns the offset location in bytes from the beg of file
	plaintextLength = lseek(file_descriptor, 0, SEEK_END);
	// Subtract 1 to strip off the last newline character
       //plaintextLength--;
	//printf("plaintextLength: %d\n", plaintextLength);
	//fflush(stdout);
	// Get current position (bytes from the beg of file)
	//plaintextLength = ftell(file_descriptor);
	// Create plaintext buffer and clear it out 
	char plaintext[plaintextLength];
	memset(plaintext, '\0', sizeof(plaintext));
	// Reset the file pointer back to the beginning
	lseek(file_descriptor, 0, SEEK_SET);
	ptCharsRead = read(file_descriptor, plaintext, plaintextLength);
	if (ptCharsRead < 0) error("ERROR: reading file");
	// Replace last /n character with \0
       plaintext[plaintextLength - 1] = '\0';
	//printf("plaintext: %s\n", plaintext);
	//fflush(stdout);
	// Close the file
	close(file_descriptor);

	// Open key file for reading
	file_descriptor = open(argv[2], O_RDONLY);
	if(file_descriptor == -1) {
            fprintf(stderr, "ERROR: cannot open %s\n", argv[1]);
            exit(1);
        }
	keyLength = lseek(file_descriptor, 0, SEEK_END);
	//keyLength = ftell(file_descriptor);
	// If key is < plaintext, error!
	if (keyLength < plaintextLength) { 
	    fprintf(stderr, "ERROR: key '%s' is too short\n", argv[2]);
	    exit(1);
	}
	char key[keyLength];
	memset(key, '\0', sizeof(key));
	lseek(file_descriptor, 0, SEEK_SET);
	keyCharsRead = read(file_descriptor, key, keyLength);
	if (keyCharsRead < 0) error("Error: reading file");
	key[keyLength - 1] = '\0'; 
	close(file_descriptor);

	// Check if plaintext or key contain bad chars
	// Good chars: ASCII 65-90 (A-Z), 32 (' ')
	int i;
	for(i = 0; i < plaintextLength - 1; i++) {
	    //printf("plaintext[i]: %d\n", plaintext[i]);
	    //fflush(stdout);
	    if(plaintext[i] > 90 || (plaintext[i] < 65 && plaintext[i] != 32)) {
		error("CLIENT: ERROR plaintext has bad characters\n");
	    }
	}	
	for(i = 0; i < keyLength - 1; i++) {
	    if(key[i] > 90 || (key[i] < 65 && key[i] != 32)) {
		error("CLIENT: ERROR key has bad characters\n");
	    }
	}	

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket- returns the fd if it succeeds or -1 if fails
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server- returns 0 on success, -1 on failure
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { // Connect socket to address
	    fprintf(stderr, "ERROR: %s could not find port %d\n ", argv[0], portNumber);
	    exit(2);	
	}

	// Send id to server (make sure we're communicating with otp_enc_d)
	charsWritten = send(socketFD, id, strlen(id), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(id)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	busySending(socketFD);
/*	
	int checkSend = -5; // Holds amount of bytes remaining in send buffer
	do {
	    ioctl(socketFD, TIOCOUTQ, &checkSend); // Check the send buffer for this socket
	    //printf("checkSend: %d\n", checkSend); // Check how many remaining bytes there are
	} while(checkSend > 0);
	if(checkSend < 0) error("ioctl error");	// Check if we actually stopped the loop because of an error
*/
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	//fflush(stdout);

	// No match, close socket, terminate
	if(strcmp(buffer, "client mismatch") == 0) {
	    fprintf(stderr, "ERROR: %s cannot use otp_dec_d\n", argv[0]);
	    close(socketFD);
	    exit(2);
	}

	// Match, so send the plaintext and key length
	charsWritten = send(socketFD, &plaintextLength, sizeof(plaintextLength), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	busySending(socketFD);
	charsWritten = send(socketFD, &keyLength, sizeof(keyLength), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	busySending(socketFD);
	// Does server need to check key length and send response???
	// I don't think so since client already did it???

	// Send the plaintext and key 
	charsWritten = send(socketFD, plaintext, sizeof(plaintext), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	busySending(socketFD);
	charsWritten = send(socketFD, key, sizeof(key), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	busySending(socketFD);
	
	// Server does encryption and sends ciphertext back
	char ciphertext[plaintextLength];
	memset(ciphertext, '\0', sizeof(ciphertext));
	charsRead = recv(socketFD, ciphertext, sizeof(ciphertext), MSG_WAITALL); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	// Print ciphertext, add newline to the end!
	//printf("CLIENT: I received this from the server: \"%s\"\n", ciphertext);
	printf("%s\n", ciphertext);
	
	close(socketFD); // Close the socket
	return 0;
}
