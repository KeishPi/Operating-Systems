/**************************************************************************************
 * Author: 	Keisha Arnold
 * Date: 	02/13/2017 CS344-400
 * Program 2: 	arnoldke.adventure.c 
 * Description: This assignment is to write a simple game akin to an old text 
 * 		adventure game like Adventure. The assignment is split up into two
 * 		programs. The first program ("rooms program") creates a series of 
 * 		files that hold descriptions of the in-game rooms and how the rooms
 * 		are connected.  This is in a file named arnoldke.buildrooms.c.
 * 		The second program ("game") provides an interface for playing the 
 * 		game using the most recently generated rooms compiled in the first
 * 		program. In the game, player will begin in the "starting room" and 
 * 		will win the game automatically upon entering the "ending room",
 * 		which causes the game to exit, displaying the path and steps taken
 * 		by the user. During the game,the player can also enter a command that 
 * 		returns the current time implemented using mutexes and multithreading.
***************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 	
#include <sys/types.h>
#include <sys/stat.h> 
#include <time.h> 
#include <fcntl.h>
#include <dirent.h> 
//#include <locale.h>
#include <pthread.h>
#include <assert.h>

#define NUM_ROOMS 7		//# of rooms in the game
#define NUM_ROOM_NAMES 10	//# of rooms to choose from
#define MIN_EDGES 3		//min # of connection
#define	MAX_EDGES 6		//max # of connections

//Create and initialize the mutex to start it running
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_init(myMutex, NULL); 

enum roomTypes {START_ROOM, MID_ROOM, END_ROOM};

struct Rooms {
    char* name;
    struct Rooms* connections[NUM_ROOMS];
    int numConnections;
    int capConnections;
    enum roomTypes type;
};

/**************************************************************************************
 * Function: 		void printRoom(struct Rooms *room, int con) 
 * Description:		This function is given a pointer to a struct Room and an int 
 * 			representing the number of connections to the room, then prints  
 * 			the contents of the Room.
 * Parameters:		struct Rooms, int
 * Pre-Conditions: 	room and con are initialized
 * Post-Conditions: 	none	 
***************************************************************************************/
void printRoom(struct Rooms *room, int conn) {
    int i;
    printf("ROOM NAME: %s\n", room->name);

    for(i = 0; i < conn; i++) {
	printf("CONNECTION %d: %s\n", i + 1, room->connections[i]->name);
    }

    //printf("NUM CONNECTIONS: \"%d\"\n", room->numConnections);
    printf("CAP CONNECTIONS: \"%d\"\n", room->capConnections);
    if(room->type == 0) {
	printf("ROOM TYPE: START_ROOM\n");
    }
    if(room->type == 1) {
	printf("ROOM TYPE: MID_ROOM\n");
    } 
    if(room->type == 2) {
	printf("ROOM TYPE: END_ROOM\n");
    } 
}


/**************************************************************************************
 * Function: 		char* findRoomsDirectory() 
 * Description:		This function returns a string to the most recently created
 * 			rooms directory in the working directory.
 * Parameters:		none
 * Pre-Conditions: 	none
 * Post-Conditions: 	none	 
***************************************************************************************/
char* findRoomsDirectory() {
    struct stat attr;
    DIR *d;
    struct dirent *dir;
    int newestFileTime = 0; //keeps track of the newest file
    int fileTime = 0;   //modified time of the current folder
    char *roomsDir;   //name of the newest directory
    //opendir() opens a directory stream corresponding to the directory name
    //and returns a pointer to the directory stream. The stream is positioned
    //at the first entry in the directory.
    d = opendir(".");
    //if(d) {
    //readdir() returns a pointer to a dirent stucture representing
    //the next directory entry in the directory stream pointed to by d
    while ((dir = readdir(d)) != NULL) { 
        //if you find a file containing the string "arnoldke.rooms." then stat it
	if(strstr(dir->d_name, "arnoldke.rooms.") != NULL) {   
            stat(dir->d_name, &attr);
	    //printf("Filename: %s\n", dir->d_name);
            //printf("Last modified time: %s\n", ctime(&attr.st_mtime));
	    fileTime = attr.st_mtime;
	    //if mod time of current file is newer than newestFileTime,
	    //update newestFileTime and save that filename
	    if (fileTime > newestFileTime) {
		newestFileTime = fileTime;
	        roomsDir = dir->d_name;
	        //printf("gameDir: %s\n", roomsDir); 
	    }
        }
    }
    closedir(d);
    if(roomsDir == NULL) {
	printf("Could not find rooms directory\n");
    }
    //d = opendir(roomsDir);
    return roomsDir;
}

/**************************************************************************************
 * Function: 		struct Rooms* readRooms (char *dirName) 
 * Description:		This function is given a directory filename, opens the directory,
 * 			opens the room files in the directory, reads the information,
 * 			and parses it into the returned array of pointers to struct rooms.
 * Parameters:		char*
 * Pre-Conditions: 	char* dirName contains the <ONID>.rooms.* directory name
 * Post-Conditions: 	none	 
***************************************************************************************/
struct Rooms* readRooms(char *dirName) {
    int rmCount = 0;
    int i;
    int j;
    int k;
    //int rmNum;	//keep track of rooms 
    char readBuffer[32];
    char *pos;
    int ch, numLines = 0;
    DIR *d;
    struct dirent *dir;

    //we will read from the files into this array of pointers to Rooms
    struct Rooms *gameRooms = malloc(NUM_ROOMS * sizeof(struct Rooms));
    //struct Rooms room;
    //switch to directory we want
    chdir(dirName); 
    d = opendir(".");
    while((dir = readdir(d)) != NULL) {
	if(strstr(dir->d_name, ".") == NULL) { //if there is a "." in the filename skip it
	    //files are named by room name, so assign name to each room
	    //do this first so we can refer to each Room
	    //printf("dirName: %s\n", dir->d_name);
	    gameRooms[rmCount].name = dir->d_name;
	    rmCount++;
	    //printf("rmCount: %d\n", rmCount);
	    //printf("Room Name: %s\n", gameRooms[rmCount].name);
	}
    }

    //int y;
    //for(y = 0; y < NUM_ROOMS; y++) {
	//printf("gameRoom Name: %s\n", gameRooms[y].name);
    //}	
    //while((dir =  readdir(d)) != NULL) {	    
    for(i = 0; i < NUM_ROOMS; i++) { 
    //printf("i: %d\n", i); 
        //struct Rooms room = gameRooms[i];
	//printf("Room name: %s\n", gameRooms[i].name);
 	    //open file
    	    FILE *file = fopen(gameRooms[i].name, "r");
    	    if(file < 0) {
        	printf("Open() failed on \" %s""\n", dir->d_name);
		exit(1);
    	    }
	    
   	     //count the lines in the file, so we know how many connections
    	     while (ch != EOF) {
        	ch = fgetc(file);
        	if(ch == '\n') {
	    	    numLines++;
        	}
    	    }
	    //set the number of connections
    	    gameRooms[i].capConnections = numLines - 2;
 	    //printf("Num Lines: %d\n", gameRooms[i].capConnections);

    	    //parse the file
    	    fseek(file, 0, SEEK_SET);
    	    //get past the first line, we want connections!
    	    fgets(readBuffer, sizeof(readBuffer), file);
    	    for(j = 0; j < gameRooms[i].capConnections; j++) {
    		//get past the first ROOM NAME: line, we want connections!
    		//fgets(readBuffer, sizeof(readBuffer), file);
    		//clear out the buffer
        	memset(readBuffer, '\0', sizeof(readBuffer));
        	//position is at beg of 2nd line move to position after (CONNECTION #: )
        	fseek(file, 14, SEEK_CUR);
		//get the connections
		fgets(readBuffer, sizeof(readBuffer), file);
		//remove the newline from the end so we get an accurate strcmp
		if((pos = strchr(readBuffer, '\n')) != NULL) {
		    *pos = '\0';
		}
		//printf("connection:%s\n", readBuffer);
 		for(k = 0; k < NUM_ROOMS; k++) {
		    //printf("connection:%s\n", gameRooms[k].name);
	            //printf("room[i]: %s\n", &room.name);
		    //while(readBuffer != NULL || room.name != NULL) {
	    	    if(strcmp(readBuffer, gameRooms[k].name) == 0) {
			gameRooms[i].connections[j] = &gameRooms[k]; 
			//printf("connection:%s\n", gameRooms[k].name);
	    	    }
		    //}
		}
    		//printf("numLines: %d\n", numLines);
            }

	    //printf("Num Lines: %d\n", gameRooms[i].capConnections);

		memset(readBuffer, '\0', sizeof(readBuffer));
 		fseek(file, 11, SEEK_CUR);
		fgets(readBuffer, sizeof(readBuffer), file);
		if(strcmp(readBuffer, "START_ROOM\n") == 0) {
		    gameRooms[i].type = START_ROOM;
		} else if (strcmp(readBuffer, "MID_ROOM\n") == 0) {
		    gameRooms[i].type = MID_ROOM;
		} else if(strcmp(readBuffer, "END_ROOM\n") == 0) {
		    gameRooms[i].type = END_ROOM;
		}
		//printf("room type: %s\n", readBuffer);  
            fclose(file);

 	    //printf("Num Lines: %d\n", gameRooms[i].capConnections);

//       	    int y, z;
//    	    for(y = 0; y < NUM_ROOMS; y++) {
//		printf("gameRoom Name: %s\n", gameRooms[y].name);
//		printf("NumCon: %d\n", gameRooms[y].numConnections);
// 		for(z = 0; z < gameRooms[y].capConnections; z++) 
//	    	    printf("Connections: %s\n", gameRooms[y].connections[z]->name);
//            }

       // }
	ch = numLines = 0;
    }

    closedir(d);
    chdir("..");
    return gameRooms;
}

/**************************************************************************************
 * Function: 		void timekeeping() 
 * Description:		This function writes the current time of day to a file in the
 * 			working directory, using a second thread. The main thread then 
 * 			reads the time from the file and prints it to stdout.
 * Parameters:		none
 * Pre-Conditions: 	none
 * Post-Conditions: 	.txt file created in working directory	 
***************************************************************************************/
void* timekeeping() {
    int file_descriptor;
    char *newFilePath = "./currentTime.txt";
    ssize_t nread, nwritten;

    time_t rawtime;
    struct tm * timeinfo;
    char timeBuf[80];
    char readBuffer[80];

    memset(timeBuf, '\0', sizeof(timeBuf));
    //get time
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    //format the time
    strftime(timeBuf, 80, "%I:%M%P, %A, %B %d, %Y", timeinfo);
    printf("\n");
    puts(timeBuf);
    //printf(timeBuf);	
    
    //open file and write time to it
    file_descriptor = open(newFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if(file_descriptor == -1) {
       	printf("open failed");
       	exit(1);
    }
    nwritten = write(file_descriptor, timeBuf, strlen(timeBuf) * sizeof(char));

    lseek(file_descriptor, 0, SEEK_SET);
    nread = read(file_descriptor, readBuffer, sizeof(readBuffer));
    //printf(readBuffer);
    pthread_exit(NULL);
    
}

/**************************************************************************************
 * Function: 		void playGame(struct Rooms *gmRooms) 
 * Description:		This function is given an array of pointers to Rooms and provides
 * 			the interface to play the game. It starts the player in the 
 * 			START_ROOM, displaying their current location and possible
 * 			connections. When the player reaches the END_ROOM it prints the
 * 			number of steps and path the user took to get there, then exits.
 * Parameters:		struct Rooms *gmRooms
 * Pre-Conditions: 	struct Rooms contains the rooms needed for play
 * Post-Conditions: 	none	 
***************************************************************************************/
void *playGame(struct Rooms *gmRooms) {
    int i, j, k, m;
    int resultCode;
    int numSteps = 0; 
    int numRoomsVisited = NUM_ROOMS;
    char *path[NUM_ROOMS * 5];  //keep track of rooms user visits
    char userBuf[24];	   //memset!
    char *pos;
    int found = 0;

    int resultInt;
    pthread_t myThreadID;

    struct Rooms *curRoom = malloc(sizeof(struct Rooms));  
    int n;
    for(n = 0; n < NUM_ROOMS + 3; n++) 
	curRoom->connections[n] = (struct Rooms*)malloc(sizeof(struct Rooms));

    //Find start room and set curRoom to that room
    for(i = 0; i < NUM_ROOMS; i++) {
	if(gmRooms[i].type == 0) {
	    curRoom = &gmRooms[i];
	    //printf("current room: %s\n", curRoom->name);
	}
    }
   
    //while user has not reached end room
    while(curRoom->type != 2) {
    	//player starts in starting room
    	found = 0; //reset found flag
    	printf("\nCURRENT LOCATION: %s\n", curRoom->name);
	printf("POSSIBLE CONNECTIONS: ");
	for(j = 0; j < curRoom->capConnections - 1; j++) {
	    printf("%s, ", curRoom->connections[j]->name);
	} //adding a period to the last room name.
	if(curRoom->capConnections > 0) {
	    printf("%s.\n", curRoom->connections[curRoom->capConnections - 1]->name);
	} 
	printf("WHERE TO? >");

	memset(userBuf, '\0', sizeof(userBuf));
	//get room name from user
	fgets(userBuf, sizeof(userBuf), stdin);
	//remove the newline from the end so we get an accurate strcmp
	if((pos = strchr(userBuf, '\n')) != NULL) {
	    *pos = '\0';
	}

	//int b;
	//for(b = 0; b < curRoom->capConnections; b++) 	
	//printf("curRoomCons:%s\n", curRoom->connections[b]->name); 

	//display current date & time 
	while(strcmp(userBuf, "time") == 0) {
	    found = 1;

	    //unlock the ***MUTEX*** originally locked in main
	    pthread_mutex_unlock(&myMutex);
    	    //create 2nd ***THREAD***
    	    resultInt = pthread_create(&myThreadID, NULL, timekeeping, NULL);  
	    //Block my main process until this thread terminates (pjoin)
	    resultCode = pthread_join(myThreadID, NULL);
	    assert(0 == resultCode);
	    //lock the mutex
   	    pthread_mutex_lock(&myMutex);
	
	    //get the time- this works without the threads/mutex
	    //timekeeping();
	    
 	    printf("\nWHERE TO? >");
	    memset(userBuf, '\0', sizeof(userBuf));
	    //get location from user
	    fgets(userBuf, sizeof(userBuf), stdin);
	    //printf("\n");
	    //remove the newline from the end so we get an accurate strcmp
	    if((pos = strchr(userBuf, '\n')) != NULL) {
	        *pos = '\0';
	    }
	}found = 0; 

    	//printf("\nCURRENT LOCATION: %s\n", curRoom->name);
    	//printf("userBuf: : %s\n", userBuf);
	//int b;
	//for(b = 0; b < curRoom->capConnections; b++) 	
	//printf("curRoomCons:%s\n", curRoom->connections[b]->name); 
 
    	//check that the location is valid
    	for(k = 0; k < curRoom->capConnections; k++) {
    	    //printf("userBuffer:%s\n", userBuf);
	    //printf("curRoomConnections:%s\n", curRoom->connections[k]->name); 
	    //if ok, move there
	    if(strcmp(userBuf, curRoom->connections[k]->name) == 0) {
		found = 1;
		curRoom = curRoom->connections[k];
	        //check if path array needs more space!!!! 
	        //if(numSteps >= numRoomsVisited) {
		//    numRoomsVisited += NUM_ROOMS;
		    //copy old array into new array
		//}   
    	        //put that location name into the path, increment steps
    	        path[numSteps] = curRoom->name;
	        numSteps++;
	    }
	    //printf("usser buf:%s\n", userBuf);
	}
	if(found == 0) {
	    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
	}
    } 
    //user has entered the end room! Game over.
    if(curRoom->type == 2) {
	printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", numSteps);
	for(m = 0; m < numSteps; m++) {
	    printf("%s\n", path[m]);
        }
    }
	
}

/**************************************************************************************
 * Function: 		void cleanUp(struct Rooms *allRooms) 
 * Description:		This function frees all remaining memory allocated during the game.
 * Parameters:		struct Rooms *allRooms
 * Pre-Conditions: 	struct Rooms contains the rooms needed for play
 * Post-Conditions: 	none	 
***************************************************************************************/
void cleanUp(struct Rooms *allRooms) {
    int i;
    //for(i = 0; i < NUM_ROOMS; i++) { 
        //free(allRooms->connections[i]);
    //}
    free(allRooms); //does this need to go in the loop too?
}


/**************************************************************************************/
int main(void) {
/*
    pthread_t myThreadID;	//stores the ID of the thread
    int resultInt;	
    
    Create and initialize the mutex to start it running
    pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
    Establish the lock
    Once locked any other thread attempting to lock it will be blocked 
    pthread_mutex_lock(&myMutex);
    Create 2nd thread for the time
    Keep this running and just invoke it as needed to get the time
    resultInt = pthread_create(&myThreadID, NULL, timekeeping, NULL);  
    Attempt to call lock, gets blocked because main has the lock right now
    pthread_mutex_lock(myMutex);
*/
	
    char* rmDir = findRoomsDirectory();
    //printf("rmDirMain: %s\n", rmDir); 

    struct Rooms *theRooms = readRooms(rmDir);
    
/*
    int i, z;
    for(i = 0; i < NUM_ROOMS; i++) {
	printf("gameRoom Name: %s\n", theRooms[i].name);
	printf("NumCon: %d\n", theRooms[i].capConnections);
 	for(z = 0; z < theRooms[i].capConnections; z++) 
	    printf("Connections: %s\n", theRooms[i].connections[z]->name);
    }
*/  
    playGame(theRooms);

    cleanUp(theRooms);
 
    return 0;
    
}


/******code that didn't quite work*******/
/*
struct Rooms getRoomInfo(char *name) {
    int i;
    int j;
    int k;
    int rmNum;	//keep track of rooms 
    char readBuffer[32];
    int ch, numLines= 0;

ye    struct Rooms rm;

    //loop through gameRooms and match it to the file name
    //for each room...look for matching file
	//for(rmNum = 0; rmNum < NUM_ROOMS; rmNum++) { 
    printf("Room Name: %s\n", rm.name);
	    //for(i = 0; i < NUM_ROOMS; i++) {
                //match found, open the file and get connections
		//if(strcmp(gameRooms[rmNum].name, dir->d_name) == 0) {
	        //open the file
    FILE *file = fopen(name, "r");
    if(file < 0) {
        printf("Open() failed on \" %s""\n", name);
	exit(1);
    }

    //count the lines in the file, so we know how many connections
    while (ch != EOF) {
        ch = fgetc(file);
        if(ch == '\n') {
	    numLines++;
        }
    }
    rm.numConnections = numLines - 2;

    //parse the file
    //get past the first line, we want connections!
    fgets(readBuffer, sizeof(readBuffer), file);
    for(j = 0; j < (numLines - 2); j++) {
    //get past the first ROOM NAME: line, we want connections!
    //fgets(readBuffer, sizeof(readBuffer), file);
    //clear out the buffer
        memset(readBuffer, '\0', sizeof(readBuffer));
        //position is at beg of 2nd line move to position after (CONNECTION #: )
        fseek(file, 14, SEEK_CUR);
	//get the connections
	fgets(readBuffer, sizeof(readBuffer), file);
	//printf("connection:%s", readBuffer);
	for(k = 0; k < NUM_ROOMS; k++) {
	    if(strcmp(readBuffer, rm.name) == 0) {
		rm.connections[j] = &gameRooms[k]; 
	    }
	}
    //printf("numLines: %d\n", numLines);
//    int z;
//    for(z = 0; z < (numLines-2); z++) {
//        printf("Connections: %s\n", gameRooms[rmNum].connections[z]->name);
//    }
//    ch = numLines = 0;
    }
    fclose(file);
    return rm;
}
*/
