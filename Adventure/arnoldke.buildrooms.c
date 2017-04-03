/*************************************************************************************
 * Author: 	Keisha Arnold
 * Date: 	02/13/2017 CS344-400
 * Program 2: 	arnoldke.buildrooms.c 
 * Description: This assignment is to write a simple game akin to an old text 
 *     		adventure game like Adventure. The assignment is split up into two
 *    		programs. The first program ("rooms program") creates a series of 
 *    		files that hold descriptions of the in-game rooms and how the rooms
 *     		are connected.  This is in a file named arnoldke.buildrooms.c.
 *     		The second program ("game") provides an interface for playing the 
 *     		game using the most recently generated rooms compiled in the first
 *     		program. In the game, player will begin in the "starting room" and 
 *     		will win the game automatically upon entering the "ending room",
 *     		which causes the game to exit, displaying the path and steps taken
 *  		by the user. During the game,the player can also enter a command that 
 * 		returns the current time implemented using mutexes and multithreading.
 *************************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> //mkdir, stat
#include <time.h> //srand
#include <fcntl.h>

#define NUM_ROOMS 7
#define NUM_ROOM_NAMES 10
#define MIN_EDGES 3
#define	MAX_EDGES 6

enum roomTypes {START_ROOM, MID_ROOM, END_ROOM};

char* roomNames[NUM_ROOM_NAMES] = {"Zork", "Cave", "Hogwarts", "Dungeon", "JUMANJI",
				   "Narnia", "zion", "PLUTO", "Atlantis", "tesseract"};
 
struct Rooms {
    char* name;
    struct Rooms* connections[NUM_ROOMS];
    int numConnections;
    int capConnections;
    enum roomTypes type;
};

//where we'll store our randomly chosen Rooms
struct Rooms roomList[NUM_ROOMS];					

/****************************************************************************************
 * Function: 		char* buildDirectoryName() 
 * Description:		This function builds up the directory name string, allocates
 *   		 	memory on the heap for it and returns a pointer to that address.   
 * Parameters:		none
 * Pre-Conditions: 	none
 * Post-Conditions: 	pointer to the address of a directory name string	 
 *****************************************************************************************/
char* buildDirectoryName() {
    pid_t myPID = getpid(); 
    char* myONID = "arnoldke";
    //char myRoomsDirName[64]; //doesn't work bc this is a local variable
    //this works because malloc returns an address on the heap,
    //which has a lifetime which spans the entire execution of the program 
    //or until you call free()
    //remember to FREE this!
    char *myRoomsDirName = malloc(64 * sizeof(char)); 
        if(myRoomsDirName == 0) 
	    printf("malloc() failed!\n");
    memset(myRoomsDirName, '\0', sizeof(myRoomsDirName));
    
    sprintf(myRoomsDirName, "./%s.rooms.%d", myONID, myPID);
    //printf("Built-up rooms dir is: \"%s\"\n", myRoomsDirName);
    return myRoomsDirName;
}

/****************************************************************************************
 * Function: 		void printRoom(struct Rooms room) 
 * Description:		This function prints the contents of a struct Rooms
 * Parameters:		struct Rooms
 * Pre-Conditions: 	none
 * Post-Conditions: 	displays the contents of the struct Rooms to stdout	 
 *****************************************************************************************/
void printRoom(struct Rooms room) {
    int i;
    printf("ROOM NAME: %s\n", room.name);

    for(i = 0; i < room.numConnections; i++) {
	printf("CONNECTION %d: %s\n", i + 1, room.connections[i]->name);
    }

    printf("NUM CONNECTIONS: \"%d\"\n", room.numConnections);
    printf("CAP CONNECTIONS: \"%d\"\n", room.capConnections);
    if(room.type == 0) {
	printf("ROOM TYPE: START_ROOM\n");
    }
    if(room.type == 1) {
	printf("ROOM TYPE: MID_ROOM\n");
    } 
    if(room.type == 2) {
	printf("ROOM TYPE: END_ROOM\n");
    } 
}

/****************************************************************************************
 * Function: 		void printRoomToFile(FILE* file, int num) 
 * Description:		This function prints the contents of a struct Rooms to a file
 * Parameters:		FILE*, int
 * Pre-Conditions: 	FILE* file initialized, num initialized to the index in the 
 *     			array of struct Rooms that the user wants to print to file.
 *  Post-Conditions: 	writes the contents of the int index (a room) to file	 
 *****************************************************************************************/
void printRoomToFile(FILE* file, int num) {
    int i;
    fprintf(file, "ROOM NAME: %s\n", roomList[num].name);

    for(i = 0; i < roomList[num].numConnections; i++) {
	fprintf(file, "CONNECTION %d: %s\n", i + 1, roomList[num].connections[i]->name);
    }

    //fprintf("NUM CONNECTIONS: \"%d\"\n", room.numConnections);
    //fprintf("CAP CONNECTIONS: \"%d\"\n", room.capConnections);
    if(roomList[num].type == 0) {
	fprintf(file, "ROOM TYPE: START_ROOM\n");
    }
    if(roomList[num].type == 1) {
	fprintf(file, "ROOM TYPE: MID_ROOM\n");
    } 
    if(roomList[num].type == 2) {
	fprintf(file, "ROOM TYPE: END_ROOM\n");
    } 
}

/****************************************************************************************
 * Function: 		struct Rooms* initRooms() 
 * Description:		This function initializes an array of struct rooms with unique
 *    			room names, their connections, and room type 
 * Parameters:		none
 * Pre-Conditions: 	global variable roomsList initialized with room names 
 * Post-Conditions: 	returns an array of pointers to struct Rooms	 
 *****************************************************************************************/
struct Rooms* initRooms() {
//void initRooms() {
    int i;
    for (i = 0; i < NUM_ROOMS; i++) {
	//set numConnections to 0
        roomList[i].numConnections = 0;
	
	//randomly set the number of outgoing connections (3-6)
	int randConnections = rand() % (MAX_EDGES - MIN_EDGES + 1) + MIN_EDGES;
        roomList[i].capConnections = randConnections;
	//printf("CapCon = \%d\"\n", roomList[i].capConnections);

	//set all the room types to MID_ROOM for now
	roomList[i].type = MID_ROOM;
	
	//clear out the name array for each Room
	//memset(&roomList[i].name, '\0', sizeof(roomList[i].name));

	//choose room names
	while (1) {
	    //generate rand number to choose room name
	    int num = rand() % NUM_ROOM_NAMES;
	    char* randName = roomNames[num];
	    //printf("randName: \"%s\"\n", roomNames[num]);

	    //check if name is already in use
	    int j = 0;
	    int found = 0;
	    //for(j = 0; j < NUM_ROOMS; j++) {
	    while(roomList[j].name != NULL) { //can't be null when using strcmp!!!
  	        if((strcmp(randName, roomList[j].name)) == 0) {  
		    found = 1;
		    //printf("randName: \"%s\"\n", randName);
		    //printf("roomList[j].name: \"%s\"\n", roomList[j].name);
		    //printf("name found: \"%s\"\n", randName);
		} 
		j++;
	    }
	    //}
	    //wasn't found so assign it to a room
	    if (found == 0) {
		roomList[i].name = randName;	
		//printf("assigning name: \"%s\"\n", roomList[i].name);
		break;
	    }
	}//printRoom(roomList[i]);
    }

    int index;
    struct Rooms *room1;
    int j;
    int k;
    //Connect the rooms
    for (i = 0; i < NUM_ROOMS; i++) {
    //we've already set the cap of connections so we know it's > 2 and  < 7
        room1 = &roomList[i];
	//printf("room1: \"%s\"\n", room1->name);

	//while(room1.numConnections < room1.capConnections) {
	for(j = 0; j < room1->capConnections; j++) { 
	    //make sure room isn't connected to itself or rooms aren not already connected
	    int flag = 0;
	    while(flag == 0) {
		index = rand() % NUM_ROOMS;
    	        //printf("rm2.numCon: \"%d\"\n", roomList[index].numConnections);
		//printf("rm2.capCon: \"%d\"\n", roomList[index].capConnections);
		if(roomList[index].numConnections >= roomList[index].capConnections) {
		    flag = -1;
		}
		//if room isn't connected to itself, or if room2 has enough connections enter here
		//otherwise get another room
		if((index != i) && (flag >= 0)) {
		    flag = 1;
		    for(k = 0; k < room1->numConnections; k++) {
			//printf("rm1.capCon: \"%d\"\n", room1->capConnections);
			//printf("rm1.connections: \"%s\"\n", room1->connections[k]->name);
    	    	        //printf("rm2.name: \"%s\"\n", roomList[index].name);
			if(strcmp(room1->connections[k]->name, roomList[index].name) == 0) {
			    flag = 0; //already connected, set flag to 0 and loop again
			}
		    }
		}
	    }
	    if(flag == 1) {
    	    //connect rooms to each other and increment numConnections
    	    room1->connections[room1->numConnections] = &roomList[index];
    	    //printf("rm1.connections: \"%s\"\n", rm1->connections[rm1->numConnections]->name);
    	    roomList[index].connections[roomList[index].numConnections] = room1;
    	    room1->numConnections++;
	    roomList[index].numConnections++;
	    }

/*
	    struct Rooms *room2 = &roomList[index];
       	    printf("room2: \"%s\"\n", room2->name);

 	    while(!connectRooms(room1, room2)) {
		index = rand() % NUM_ROOMS;
	    } 
*/
	} 
    }
    //Assign the start and end rooms
    roomList[0].type = START_ROOM;
    roomList[NUM_ROOMS - 1].type = END_ROOM;
    return roomList;
}

/****************************************************************************************
 * Function: 		void roomsToFile() 
 * Description:		This function prints the contents of an array of struct Rooms to
 *    			a their own file titled as their room name
 * Parameters:		none
 * Pre-Conditions: 	global variable roomList initialized with struct Rooms 
 * Post-Conditions: 	writes the contents of the roomList array to files
 *****************************************************************************************/
void roomsToFile() {
    int i;
    //FILE *file_descriptor;
    //char* file = roomList[i].name;
    
    for(i = 0; i < NUM_ROOMS; i++) {
    //file_descriptor = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0600);

        FILE *file_descriptor;
        char* file = roomList[i].name;

        //open the file
        file_descriptor = fopen(file, "w");
        //check if successful
        if(file_descriptor == NULL) {
	    printf("open() failed on \"%s\"\n", file);
	    exit(1);
        }
        //write to file 
        printRoomToFile(file_descriptor, i);
        //do I need to leseek to the beginning of the file???
        //close the file
        fclose(file_descriptor);
    }
}

/*************************************************************************/
void main() {
    time_t t; //time to seed srand
    srand((unsigned) time(&t));
    
    char *dirName = buildDirectoryName();
    mkdir(dirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    //printf("dirName is: \"%s\"\n", dirName);
 
    initRooms();   
    
    //int i;
    //for(i = 0; i < NUM_ROOMS; i++) {
   	//printRoom(roomList[i]);
    //}

    chdir(dirName);

    roomsToFile();
   
    chdir(".."); 
 
    free(dirName);  
}




//Code that I tried, but didn't work :(
/*
int connectRooms(struct Rooms *rm1, struct Rooms *rm2) {
   printf("rm1Num: \"%d\"\n", rm1->numConnections);
   printf("rm1Cap: \"%d\"\n", rm1->capConnections);
   printf("rm2Num: \"%d\"\n", rm2->numConnections);
   printf("rm2Cap: \"%d\"\n", rm2->capConnections);

//   if((rm1->numConnections >= rm1->capConnections) ||
//      (rm2->numConnections >= rm2->capConnections)) {
//      printf("can't connect- cap reached");
//        return 0; //can't connect- capacity reached
//    }
    if(rm1->numConnections == MAX_EDGES) {
	printf("max edges reached");
        return 1; //return true- go to next room
    }
    if(rm1->numConnections >= MAX_EDGES || rm2->numConnections >= MAX_EDGES) {
	printf("2 max edges reached");
        return 0; //return false
    }
    if(checkConnections(rm1, rm2)) {
	return 0; //rooms are already connected
    }
    
    //connect rooms to each other and increment numConnections
    rm1->connections[rm1->numConnections] = rm2;
    //printf("rm1.connections: \"%s\"\n", rm1->connections[rm1->numConnections]->name);
    rm2->connections[rm2->numConnections] = rm1;
    rm1->numConnections++;
    rm2->numConnections++;
    //printf("ok connection made!");
    return 1;
} 


int checkConnections(struct Rooms *r1, struct Rooms *r2) {
    //connects to itself
//    while(rm1.name != NULL && rm2.name != NULL) { //can't be null when using strcmp!!!
//        if((strcmp(rm1.name, rm2.name)) == 0) {
//	    return 1; 
//        }
//    }
//
//    printf("rm1.name: \"%s\"\n", rm1.name);
//    printf("rm2.name: \"%s\"\n", rm2.name);
    
   printf("rm1Num: \"%d\"\n", r1->numConnections);
   printf("rm1Cap: \"%d\"\n", r1->capConnections);
   printf("rm2Num: \"%d\"\n", r2->numConnections);
   printf("rm2Cap: \"%d\"\n", r2->capConnections);
    //printf("rm1.connections: \"%d\"\n", rm1.capConnections);
    int i;
    for(i = 0; i < r1->numConnections; i++) {
      printf("i: \"%d\"\n", i);
      printf("r1.connections->name: \"%s\"\n", r1->connections[i]->name);
   //     while(rm1.connections[i]->name != NULL) {
//	    printf("hello");
	    if((strcmp(r1->connections[i]->name, r2->name)) == 0) {
	        return 1;
            }
//        }
    }
    return 0; //not connected- all good!
}
*/
