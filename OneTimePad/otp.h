/*****************************************************************************************
 * Author:      Keisha Arnold
 * Date:        03/17/2017 CS344-400
 * Program 4:   otp.h 
 * Description: otp.h contains the function declarations used in otp_enc_d.c, otp_enc.c, 
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


/*******************************************************************************************
* Function:             void error(const char *msg) 
* Description:          Error function used for reporting issues and sets the exit value
*                       to 1
* Parameters:           const char *msg
* Pre-Conditions:       none
* Post-Conditions:      displays error msg and exits  
********************************************************************************************/
void error(const char *msg);

/*******************************************************************************************
* Function:             int checkClient(char* buffer, int establishedConnectionFD, 
*                                       char* client) 
* Description:          checkClient function makes sure the server is communicating with
*                       the correct client 
* Parameters:           char* buffer, int establishedConnectionFD, char* client
* Pre-Conditions:       client is a string naming the correct client, socket is set up
*                       and listening for connections
* Post-Conditions:      returns 1 if the client is a match, and 0 if there is a mismatch 
********************************************************************************************/
int checkClient(char* buffer, int establishedConnectionFD, char* client);

/*******************************************************************************************
* Function:             void doEncryption(char* ciphertext, char* plaintext, 
*                                          int plaintextLength, char* key) 
* Description:          doEncryption function encrypts the plaintext into ciphertext using
*                       a key and modular addition
* Parameters:           char* ciphertext, char* plaintext, int plaintextLength, char* key
* Pre-Conditions:       all parameters initialized
* Post-Conditions:      returns ciphertext  
********************************************************************************************/
//void doEncryption(char* ciphertext, char* plaintext, int plaintextLength, char* key);
void doEncryption(char* ciphertext, char* plaintext, char* key);

/*******************************************************************************************
* Function:             void busySending(int establishedConnectionFD) 
* Description:          busySending function puts the program in a busy loop waiting for 
*                       the network output sending buffers to clear once this function
*                       returns we know it's safe to execute the next operation and solves
*                       synchronization issues between server and client
* Parameters:           int establishedConnectionFD
* Pre-Conditions:       socket is set up and listening
* Post-Conditions:      none
********************************************************************************************/
void busySending(int establishedConnectionFD);

/*******************************************************************************************
* Function:             void doDecryption(char* plaintext, char* ciphertext, 
*                                          int ciphertextLength, char* key) 
* Description:          doDecryption function decrypts the ciphertext into plaintext using
*                       a key and modular subtraction
* Parameters:           char* plaintext, char* ciphertext, int ciphertextLength, char* key
* Pre-Conditions:       all parameters initialized, key is not shorter than ciphertext
* Post-Conditions:      returns plaintext  
********************************************************************************************/
//void doDecryption(char* plaintext, char* ciphertext, int ciphertextLength, char* key);
void doDecryption(char* plaintext, char* ciphertext, char* key);


