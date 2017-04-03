/*****************************************************************************************
* Author: 	Keisha Arnold
* Date: 	03/03/2017 CS344-400
* Program 2: 	smallsh.c 
* Description: 	The goal of this assignment is to emulate a bash shell in C. The shell
* 		will run command line instructions and return the results, similar to
* 		bash but without many of their fancier features. It will allow for the 
* 		redirection of standard input and output and will support both foreground
* 		and background processes. 
******************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 	
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <signal.h>
#include <time.h> 
#include <fcntl.h>
#include <dirent.h>

#define MAX_CHARS 2048
#define MAX_ARGS 512

char buffer[MAX_CHARS]; //command line must support a max length of 2048 chars
char* args[MAX_ARGS];	//command line must support a max of 512 arguments
int bkground = 0;	//flag if command is to be executed in the background
int argCount = 0;
int quit = 0; 		//flag if user enters "exit" on the command line
char exitStatus[280];
int SIGTSTPflag = 0;

/*******************************************************************************************
* Function: 		char getUserInput() 
* Description:		This function displays the prompt and replaces the newline character
* 			and the ampersand character with a null terminator. If it finds an
* 			ampersand character it sets the background flag.  
* Parameters:		buffer is a global variable, bkground is a global variable
* Pre-Conditions: 	none
* Post-Conditions: 	returns the buffer	 
*******************************************************************************************/
char* getUserInput() {
    char *pos;
    int len;

    //fflush(stdout);
    printf(": ");
    fflush(stdout);
    //fflush(stdin);
    memset(buffer, '\0', sizeof(buffer));	  

    if(fgets(buffer, sizeof(buffer), stdin) != NULL) {
	//replace the newline with a null terminator
	if((pos = strchr(buffer, '\n')) != NULL) {
	    *pos = '\0';
	}
	//replace the "&" at the END with a null terminator and set bkground flag
	len = strlen(buffer);
	if((strcmp(&buffer[len - 1], "&") == 0)) {
	    buffer[len - 1] = '\0';
	    //if((pos = strchr(buffer, '&')) != NULL) {	
	        //*pos = '\0';
	        bkground = 1;
	    //}
	    //if user entered ^Z then only do commands in the fg
	    if(SIGTSTPflag == 1) {
		bkground = 0;
       	    }
	}
	
/*  //fixed this above bc if the & is in the middle we want to ignore it
	if((pos = strchr(buffer, '&')) != NULL) {	
	    *pos = '\0';
	    bkground = 1;
	    //if user entered ^Z then only do commands in the fg
	    if(SIGTSTPflag == 1) {
		bkground = 0;
       	    }
	} 
*/
    }
    return buffer;
}

/*******************************************************************************************
* Function: 		void parseBuffer() 
* Description:		This function parses the buffer with strtok. Separating the commands
* 			into indexes in the args array.
* Parameters:		buffer is a global variable, args is a global variable
* Pre-Conditions: 	none
* Post-Conditions: 	args initialized with commands inputted into the buffer	 
*******************************************************************************************/
void parseBuffer() {//char* parseBuffer() {
    char* token = 0;
    argCount = 0;
    int i;
    int pid = getpid();
    char *temp;
    char *expString;
    int fnd;

    char shellPID[15];
    memset(shellPID, '\0', sizeof(shellPID));
    //convert pid from int to string
    sprintf(shellPID, "%d", pid);
    //printf("shellPID: %s\n", shellPID);
    //fflush(stdout);

    memset(args, '\0', sizeof(*args) * MAX_ARGS);
    
    for(token = strtok(buffer, " "); token != NULL; token = strtok(NULL, " ")) {
        //printf("token: %s\n", token);
        args[argCount] = token;
        //printf("args[ct]: %s\n", args[argCount]);
	argCount++;
    }
    //printf("%d\n", argCount);

    //find any instances of "$$" and replace with pid
    for(i = 0; i < argCount; i++) {
	fnd = 0;
  	char *pos = strstr(args[i], "$$");
	//printf("pos: %s\n", pos);
	//fflush(stdout);
	    while(pos != NULL) {
		fnd = 1;
		*pos = '\0';
		//printf("pos1: %s\n", pos);
		//fflush(stdout);
		//save the string w/out the $$ into temp
 		temp = args[i];
		//printf("temp: %s\n", temp);
		//fflush(stdout);
	  	break;
	    }
	//int lenTemp = strlen(shellPID);
	//printf("strlen temp: %d\n", lenTemp);
	//fflush(stdout);
	if (fnd == 1) {
	    //pid = getpid();
            //sprintf(shellPID, "%d", pid);
            //printf("shellPID: %s\n", shellPID);
            expString = malloc((strlen(temp) + strlen(shellPID) + 1) * sizeof(char));
	    if(expString == 0) {
	        printf("malloc() failed\n");
	        fflush(stdout);
	    }
	    memset(expString, '\0', sizeof(expString));
	    sprintf(expString, "%s%s", temp, shellPID);
	    //printf("ExpandedString: %s\n", expString);
	    //fflush(stdout);
	    //memset(args[i], '\0', sizeof(args[i]));
	    args[i] = expString;
	    //printf("i: %d\n", i);
	    //fflush(stdout);
	    //printf("args[i]: %s\n", args[i]);
	    //fflush(stdout);
    	}
    }
    //free(expString);
}

/*******************************************************************************************
* Function: 		void builtInCmds() 
* Description:		This function checks if the command is a built-in and executes it.
* Parameters:		buffer is a global variable, args is a global variable
* Pre-Conditions: 	none
* Post-Conditions: 	none 
*******************************************************************************************/
int builtInCmds() {
    int flag = 0;
    char cwd[1024];
    char cdPath[1024];
    int termStatus;

    if(strcmp(args[0], "exit") == 0) {
        //FIX ME- kill any process or jobs before it terminates
        //printf("exiting...");
	//fflush(stdout);
	pid_t cleanExit = waitpid(-1, &termStatus, WNOHANG);
	while (cleanExit != -1) {
	    kill(cleanExit, SIGKILL);
	    cleanExit = waitpid(-1, &termStatus, WNOHANG);
	}
        printf("exiting...");
	fflush(stdout);
	flag = 1;
	exit(0);
    }
    else if(strcmp(args[0], "status") == 0) {
	//prints the exit status OR terminating signal of last foreground process
	//printf("exit value %d\n", exitStatus);
	fflush(stdout);
	printf(exitStatus);
	fflush(stdout);
	flag = 1;
    }
    else if(strcmp(args[0], "cd") == 0) {
	if(args[1] == NULL) {	//only "cd" was entered
	    //memset(home, '\0', sizeof(home));
            char *home = getenv("HOME");
     	    //printf("home: %s\n", home);
            chdir(home); 
	    getcwd(cwd, sizeof(cwd));
	    //printf("current working dir: %s\n", cwd);
	    //fflush(stdout);
	    flag = 1;
	}
	else {	//cd [arg1] was entered
            memset(cdPath, '\0', sizeof(cdPath));
	    strcpy(cdPath, args[1]);
	    //printf("cdPath: %s\n", cdPath);
	    //fflush(stdout);
	    chdir(cdPath);	     	    
	    getcwd(cwd, sizeof(cwd));
	    //printf("current working dir: %s\n", cwd);
	    //fflush(stdout);
	    flag = 1;
	}     
    }
    return flag;
}     	

/*******************************************************************************************
* Function: 		void catchSIGTSTP(int signo) 
* Description:		Signal handler for SIGTSTP (^Z)
* Parameters:		int (signal #)
* Pre-Conditions: 	none
* Post-Conditions: 	catches the SIGTSTP signal and changes state depending on the 
* 			SIGTSTPflag	 
*******************************************************************************************/
void catchSIGTSTP(int signo) {
    if(SIGTSTPflag == 0) {
        char *message = "Entering foreground-only mode (& is now ignored)\n";
        write(STDOUT_FILENO, message, 49);
	SIGTSTPflag = 1;
    }
    else if(SIGTSTPflag == 1) {
        char *message = "Exiting foreground-only mode\n";
        write(STDOUT_FILENO, message, 29);
	SIGTSTPflag = 0;
    }
    else {
        char *message = "SIGTSTP error!\n";
        write(STDOUT_FILENO, message, 15);
    }
}

/******************************************************************************************/
int main(int argc, char *argv[]) {
    pid_t spawnpid = -5;
    int childExitStatus = -5;
    pid_t bgSpawnPid = -5;
    int bgChildExitStatus = -5;
    int i,j;
    int redirect = 0;
    int bgProcs[160] = {0};
    int bgProcCount = 0;

    //signal handlers for SIGINT
    struct sigaction dfl_action = {0}, ignore_action = {0};
    dfl_action.sa_handler = SIG_DFL;	//default for fg processes
    ignore_action.sa_handler = SIG_IGN; //ignore for bg processes

    //signal handlers for SIGTSTP
    struct sigaction TSTP_action = {0};
    TSTP_action.sa_handler = catchSIGTSTP;

    //we want the shell itself to ignore SIGINT signals
    sigaction(SIGINT, &ignore_action, NULL);
    //memset(exitStatus, '\0', sizeof(exitStatus));

    sigaction(SIGTSTP, &TSTP_action, NULL);
    
    while(quit == 0) {
	//check if any background procs have completed and print its pid and exit status
	pid_t bgZombie;
	int numZombies = 0;
	//printf("bgProcCount: %d\n", bgProcCount);
	fflush(stdout);
	for(j = 0; j < bgProcCount; j++) {
	    //while(waitpid(bgProcs[j], &bgChildExitStatus, WNOHANG) > 0) {
	    bgZombie = waitpid(bgProcs[j], &bgChildExitStatus, WNOHANG);
	    //printf("bgZombie: %d\n", bgZombie);
	    //fflush(stdout);
	    if(bgZombie > 0) {
	    numZombies++;
	    //waitpid(bgProcs[j], &bgChildExitStatus, WNOHANG);
		fflush(stdout);
		printf("background pid %d is done: ", bgZombie);
		fflush(stdout);
		//checking Normal Termination
                if(WIFEXITED(bgChildExitStatus)) {
                    printf("exit value %d\n", WEXITSTATUS(bgChildExitStatus));
                    fflush(stdout); 
		}
		//checking Signal Termination
		else if(WIFSIGNALED(bgChildExitStatus)) {
                    printf("terminated by signal %d\n",  WTERMSIG(bgChildExitStatus));
	   	    fflush(stdout);
		}
		//else {
		    //printf("what the pid: %d\n", waitpid(bgProcs[j], &bgChildExitStatus, WNOHANG));
		    //fflush(stdout);
		//}	
	    }
 	}
	bgProcCount -= numZombies;
 
        //keep prompting if there is no input or input is a comment 
        do {
	    getUserInput();
        } while(buffer[0] == '\0' || buffer[0] == '#');
        //printf("buffer: %s\n", buffer);

        //parse the buffer
        parseBuffer();
        //int i;
	//for(i = 0; i < argCount; i++) {
	    //printf("all args: %s\n", args[i]);
	//}

	//if command is a built-in 
        if(builtInCmds() == 1) {	
	    bkground = 0;
	}

	//FOREGROUND process and not built-in
	else if(bkground == 0) {
  	    spawnpid = fork();
	    switch(spawnpid) { 
	        case -1: {  //error!
		    perror("Fork Error!");
		    exit(1);
		    break;
	        }
	        case 0: { //the child is the process that's currently running
		    //before this, SIGINT gets ignored, but we want fg procs & their children 
		    //to terminate so set default SIGINT
		    sigaction(SIGINT, &dfl_action, NULL);

		    //check for redirection 
 		    for(i = 0; i < argCount; i++) {
		        //redirect = 0;
			//redirecting stdout to a file
			//printf("argCount: %d\n", argCount);
			//fflush(stdout);
	    		//printf("all args: %s\n", args[i]);
			//fflush(stdout);

	 		if(strcmp(args[i], ">") == 0) {
			    redirect = 1;
		 	    //printf("args[i + 1]: %s\n", args[i + 1]);
			    //fflush(stdout);
			    //open file for wr only, truncate it if exits, create it if not
			    int file_descriptor = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			        if(file_descriptor == -1) {
				    printf("Cannot open() %s for output", args[i + 1]);
				    fflush(stdout);
				    exit(1);
			        }
			    //call dup2() to change stdout to point where file_descriptor points
			    int result = dup2(file_descriptor, 1);
		 	    //printf("file_descriptor == %d\n");
			    //fflush(stdout);
			    if(result == -1) { 
			    	perror("dup2 error\n"); 
			    	exit(2); 
			    }
		 	    //printf("file_descriptor == %d, result == %d\n", file_descriptor, result);
			    //fflush(stdout);
			  // close(file_descriptor);
			    //redirect = 1;
			    //execvp(args[0], args);
			 //  execlp(args[i - 1], args[i - 1], NULL);
			}
			//printf("redirect: %d\n", redirect);
			//fflush(stdout);
		   // }	

	 		if(strcmp(args[i], "<") == 0) {
			    redirect = 1;
		 	    //printf("args[i + 1]: %s\n", args[i + 1]);
			    //fflush(stdout);
			    //open file for reading only
			    int file_descriptor = open(args[i + 1], O_RDONLY);;
			        if(file_descriptor == -1) {
				    printf("Cannot open %s for input\n", args[i + 1]);
				    fflush(stdout);
				    exit(1);
				    redirect = 1;
			        }
			    //call dup2() to change stdin to point where file_descriptor points
			    int result = dup2(file_descriptor, 0);
			    if(result == -1) { 
			    	perror("dup2 error\n"); 
			    	exit(2); 
			    }
		 	    //printf("file_descriptor == %d, result == %d\n", file_descriptor, result);
			    //fflush(stdout);
			  // close(file_descriptor);
			  //  execlp(args[i - 1], args[i - 1], NULL);
			}
			//printf("redirect: %d\n", redirect);
			//fflush(stdout);
		    }	
		    if(redirect == 1) {
		        execlp(args[0], args[0], NULL);
		    }
		    if(redirect == 0) {	
		        //printf("CHILD(%d): Converting into %s\n", getpid(), args[0]);
			//fflush(stdout);
		        execvp(args[0], args);
		    }
		    //if child reaches this point, then execlp must have failed
		    printf("%s could not be executed\n", args[0]);
		    fflush(stdout);
		    exit(1);
		    break;
	        }
	        default: { //the parent is the process that's currently running
		    int status;
	  	    //printf("PARENT(%d): Waiting for child(%d) to terminate\n", getpid(), spawnpid);
		    //fflush(stdout);
		    pid_t actualPid = waitpid(spawnpid, &childExitStatus, 0); 
		    //checking Normal Termination
		    if(WIFEXITED(childExitStatus)) {
			//printf("The process exited normally\n");
			//fflush(stdout); 	
		       // exitStatus = WEXITSTATUS(childExitStatus);
		        status = WEXITSTATUS(childExitStatus);
		 	sprintf(exitStatus, "exit value %d\n", status);
			//printf("Exit status was %d\n", exitStatus);
		        //fflush(stdout);
		    }
		    //checking Signal Termination
		    else if(WIFSIGNALED(childExitStatus)) {
			//printf("The process was terminated by a signal\n");
			//fflush(stdout);
		       // exitStatus = WTERMSIG(childExitStatus);
			sprintf(exitStatus, "terminated by signal %d\n", WTERMSIG(childExitStatus));
			//printf("terminated by signal %d\n", WTERMSIG(childExitStatus));
			printf(exitStatus);
			fflush(stdout);
		    }	
		    //printf("PARENT(%d): Child(%d) terminated, Exiting!\n", getpid(), actualPid);
		    //fflush(stdout);
		    break;
	        }
	    }
	}
	//BACKGROUND COMMAND
	else if(bkground == 1) {
  	    bgSpawnPid = fork();
	    switch(bgSpawnPid) { 
	        case -1: {  //error!
		    perror("Fork Error!");
		    exit(1);
		    break;
	        }
	        case 0: { //the child is the process that's currently running
		    //check for redirection 
 		    for(i = 0; i < argCount; i++) {
			//printf("argCount: %d\n", argCount);
		   	//fflush(stdout);
	    		//printf("all args: %s\n", args[i]);
			//fflush(stdout);

	 		if(strcmp(args[i], ">") == 0) {
			    redirect = 1;
		 	    //printf("args[i + 1]: %s\n", args[i + 1]);
			    //fflush(stdout);
			    //open file for wr only, truncate it if exits, create it if not
			    int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			        if(fd == -1) {
				    printf("Cannot open() %s for output\n", args[i + 1]);
				    fflush(stdout);
				    exit(1);
			        }
			    //call dup2() to change stdout to point where fd points (/dev/null)
			    int res = dup2(fd, 1);
		 	    //printf("fd == %d\n");
			    //fflush(stdout);
			    if(res == -1) { 
			    	perror("dup2 error\n"); 
			    	exit(2); 
			    }
		 	}
	 		if(strcmp(args[i], "<") == 0) {
			    redirect = 1;
		 	    //printf("args[i + 1]: %s\n", args[i + 1]);
			    //fflush(stdout);
			    //open file for reading only
			    int fd = open(args[i + 1], O_RDONLY);;
			        if(fd == -1) {
				    printf("Cannot open %s for input\n", args[i + 1]);
				    fflush(stdout);
				    exit(1);
				    redirect = 1;
			        }
			    //call dup2() to change stdin to point where fd points (/dev/null)
			    int res = dup2(fd, 0);
			    if(res == -1) { 
			    	perror("dup2 error\n"); 
			    	exit(2); 
			    }
			}
		    }
		    if(redirect == 1) { //there was redirection, exec a new proc
		        execlp(args[0], args[0], NULL);
		    }
		    if(redirect == 0) {  //no redirection, redirect stdin/stdout to /dev/null	
		    //printf("CHILD(%d): Converting into %s\n", getpid(), args[0]);
	            //fflush(stdout);
		    //printf("args[0]: %s\n", args[0]);
		    //fflush(stdout);
		    // open /dev/null for reading and writing
		    int fd = open("/dev/null", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			if(fd == -1) {
			    printf("Cannot open /dev/null for input\n");
			    fflush(stdout);
			    exit(1);
			}
		    int res =  dup2(fd, 0);	//set stdin to point to /dev/null
			if(res == -1) { 
		      	    perror("dup2 error\n"); 
			    exit(2); 
			}
		    res =  dup2(fd, 1);		//set stdout to point to /dev/null
			if(res == -1) { 
		      	    perror("dup2 error\n"); 
			    exit(2); 
			}
		    // execvp now starts with stdin and stdout pointing to /dev/null
		    execvp(args[0], args);
		    break;
		    }
		    //if child reaches this point, then execlp must have failed
		    printf("%s could not be executed\n", args[0]);
		    fflush(stdout);
		    exit(1);
		    break;
	        }
	        default: { //the parent is the process that's currently running
	  	    printf("background pid is %d\n", bgSpawnPid);
		    fflush(stdout);
	  	    //printf("PARENT(%d): Waiting for child(%d) to terminate\n", getpid(), bgSpawnPid);
		    //fflush(stdout);
		    //add this background process to our bg process array
		    bgProcs[bgProcCount] = bgSpawnPid;
		    bgProcCount++;
		    //check for completed background processes so they can be cleaned up later
		    pid_t bgActualPid  = waitpid(bgSpawnPid, &bgChildExitStatus, WNOHANG); 
		    //int reslt  = waitpid(bgSpawnPid, &bgChildExitStatus, WNOHANG); 
		   //printf("Reslt: %d\n", bgActualPid);
		    //printf("bgProcCount: %d\n", bgProcCount);
		    fflush(stdout);		    
		    break;
		}
	    } 
	}  //end of else if(bkground == 1)
	bkground = 0;
	argCount = 0;
    } //end of while(quit == 0)
    return 0;
}
