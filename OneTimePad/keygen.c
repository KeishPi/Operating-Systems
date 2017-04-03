/*****************************************************************************************
 * Author: 	Keisha Arnold
 * Date: 	03/17/2017 CS344-400
 * Program 4: 	keygen.c 
 * Description: This program creates a key file of specified length to be used in the 
 * 		one-time pad encryption/decryption.  The characters in the file generated
 * 		will be any of the 27 allowed characters (A-Z and ' '). The final character
 * 		in the key file will be the newline character.
 * ******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LO 64	//ASCII 64 (@)- 1 before A to account for space
#define HI 90	//ASCII 90 (Z)

int main(int argc, char* argv[]) {
    time_t t;		//time to seed rand
    int randChar;	//random character
    int keyLen;		//keylength inputted on command line
    int i;

    // validate the # of arguments
    if(argc < 2) {
	fprintf(stderr, "USAGE: %s keylength\n", argv[0]);
	exit(1);
    }

    // seed rand
    srand((unsigned int) time(&t));

    // 2nd arg should be the keylength- convert from string to int
    keyLen = atoi(argv[1]);
 
    //generate random characters
    //ASCII A-Z is #65-90
    //use ASCII #64 (@) as our space character
    for(i = 0; i < keyLen; i++) {
        randChar = LO + rand() % (HI - LO);
	    if(randChar == 64) {
		randChar = 32;	//ASCII 32- space character	
	    }
	    //sending to stdout but keygen # > file will send to file
	    fprintf(stdout, "%c", randChar);	
    }
    //don't forget the final newline character!
    //so if user inputs keygen 256 > mykey, mykey will actually have 257 characters
    fprintf(stdout, "\n");
    
    return 0;
}
