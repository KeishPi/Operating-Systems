/*****************************************************************************************
 * Author:      Keisha Arnold
 * Date:        03/17/2017 CS344-400
 * Program 4:   otp.c 
 * Description: otp.c contains the function definitions used in otp_enc_d.c, otp_enc.c, 
 * 		otp_dec_d.c and otp_dec.c which encrypt and decrypt information using a 
 * 		one-time pad-like system.   
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

/*******************************************************************************************
* Function: 		void error(const char *msg) 
* Description:		Error function used for reporting issues and sets the exit value
* 			to 1
* Parameters:		const char *msg
* Pre-Conditions: 	none
* Post-Conditions: 	displays error msg and exits  
********************************************************************************************/
void error(const char *msg) {
    //perror(msg);  // This produces a display bug ":Success" on the command line...why???
    		    // http://stackoverflow.com/questions/33150881/why-success-is-an-error-for-bash
    fprintf(stderr, msg);
    exit(1);
}

/*******************************************************************************************
* Function: 		int checkClient(char* buffer, int establishedConnectionFD, 
* 					char* client) 
* Description:		checkClient function makes sure the server is communicating with
* 			the correct client 
* Parameters:		char* buffer, int establishedConnectionFD, char* client
* Pre-Conditions: 	client is a string naming the correct client, socket is set up
* 			and listening for connections
* Post-Conditions:	returns 1 if the client is a match, and 0 if there is a mismatch 
********************************************************************************************/
int checkClient(char* buffer, int establishedConnectionFD, char* client) {
    // Make sure it is communicating with otp_enc
    memset(buffer, '\0', sizeof(buffer));
    int charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
    if (charsRead < 0) {
        error("ERROR reading from socket\n");
    }
    //printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    //fflush(stdout);

    // Client is not otp_enc
    if(strcmp(buffer, client) != 0) {
        return 0;
    }
    else { // Client is otp_enc
	return 1;
    }
}

/*******************************************************************************************
* Function: 		void doEncryption(char* ciphertext, char* plaintext, 
* 					   int plaintextLength, char* key) 
* Description:		doEncryption function encrypts the plaintext into ciphertext using
* 			a key and modular addition
* Parameters:		char* ciphertext, char* plaintext, int plaintextLength, char* key
* Pre-Conditions: 	all parameters initialized
* Post-Conditions: 	returns ciphertext  
********************************************************************************************/
//void doEncryption(char* ciphertext, char* plaintext, int plaintextLength, char* key) {
void doEncryption(char* ciphertext, char* plaintext, char* key) {
    //memset(ciphertext, '\0', sizeof(ciphertext));
    // Good chars - space and all cap letters
    char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int i, p, k, c;
    int textLength = strlen(plaintext);
    
    //for(i = 0; i < plaintextLength; i++) {
    // Match plaintext and key letter to it's index in the alphabet
    for(i = 0; i < textLength; i++) {
        if(plaintext[i] == ' ') {  // space is index 26 in chars
            p = 26;
        }
        else {
            p = plaintext[i] - 65;  // sub 65 to get it to the right index, since ASCII
        }			    // starts at 65 but we want A=0, B=1, C=2, etc
        if(key[i] == ' ') {
            k = 26;
        }
        else {
            k = key[i] - 65;
        }
        c = p + k;  // add plaintext and key
        c %= 27;    // mod 27 so if the number is > 27 instead of going past ' ' it will
		    // loop back around and start at A again
        ciphertext[i] = chars[c];
    }
}

/*******************************************************************************************
* Function: 		void busySending(int establishedConnectionFD) 
* Description:		busySending function puts the program in a busy loop waiting for 
* 			the network output sending buffers to clear once this function
* 			returns we know it's safe to execute the next operation and solves
* 			synchronization issues between server and client
* Parameters:		int establishedConnectionFD
* Pre-Conditions: 	socket is set up and listening
* Post-Conditions: 	none
********************************************************************************************/
void busySending(int establishedConnectionFD) {
    int checkSend = -5; // Holds amount of bytes remaining in send buffer
    do {
        ioctl(establishedConnectionFD, TIOCOUTQ, &checkSend); // Check the send buffer for this socket
        //printf("checkSend: %d\n", checkSend); // Check how many remaining bytes there are
    } while(checkSend > 0);

    if(checkSend < 0) error("ioctl error"); // Check if we actually stopped the loop because of an error
}

/*******************************************************************************************
* Function: 		void doDecryption(char* plaintext, char* ciphertext, 
* 					   int ciphertextLength, char* key) 
* Description:		doDecryption function decrypts the ciphertext into plaintext using
* 			a key and modular subtraction
* Parameters:		char* plaintext, char* ciphertext, int ciphertextLength, char* key
* Pre-Conditions: 	all parameters initialized, key is not shorter than ciphertext
* Post-Conditions: 	returns plaintext  
********************************************************************************************/
//void doDecryption(char* plaintext, char* ciphertext, int ciphertextLength, char* key) {
void doDecryption(char* plaintext, char* ciphertext, char* key) {
    //memset(plaintext, '\0', sizeof(plaintext));
    // Good chars - space and all cap letters
    char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    int i, p, k, c;
    int textLength = strlen(ciphertext);

    // Match ciphertext and key letter to it's index in the alphabet
    for(i = 0; i < textLength; i++) {
        if(ciphertext[i] == ' ') {  // space is index 26 in chars
            c = 26;
        }
        else {
            c = ciphertext[i] - 65;  // sub 65 to get it to the right index, since ASCII
        }			     // starts at 65 but we want A=0, B=1, C=2, etc
        if(key[i] == ' ') {
            k = 26;
        }
        else {
            k = key[i] - 65;
        }
	// subtract key from ciphertext
        p = c - k;
	// Need to account for negative numbers!!!- add 27 to make it positive
	// We want it to loop back around to A again...
	if(p < 0) {
	    p += 27;
	}
        p %= 27;
        plaintext[i] = chars[p];
    }
}

