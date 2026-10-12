#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50 
static char* buffer[BUFFER_SIZE]; /** Data structure that stores 10 most recent commands */
int bufferIndex = 0; /* Index of where to place next command in buffer */
int numberOfCommands = 0; /* Total number of commands entered by the user */
int sig = 0; /* Int used to track status of signal, if 1, signal was just triggered */



/* setup() reads the next command line string stored in inputBuffer, separating
   it into distinct tokens using whitespace as delimiters. setup() modifies the
   args parameter so it stores pointers to the null-terminated strings, which
   are tokens in the most-recently-entered user command line. After these null-
   terminated strings, setup() adds a final NULL pointer that marks the end of
   the list of command tokens.

   DON'T modify the setup() function for Lab 1. You WILL modify it in Lab 3,
   which builds on Lab 1. 
*/


/* the signal handler function */
void handle_SIGINT() {
// *params: file descriptor   buffer  length
    //write(STDOUT_FILENO,buffer,strlen(buffer));  
    sig = 1;      
}


void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */

    ct = 0;

    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE); // this is where it waits for user input

    if (sig == 1) { /* Checks is signal handler just ran */

        /*Print 10 most recent commands*/

        for (int i = 0; i < 10; i++) {

                int commandNum; /* Calculates number to print before command */
                int shift = numberOfCommands - 10 + 1; /* Calculates difference b/w array index and command number*/

                if (shift > 0){ /* If more than 10 total commands, array index + 1 and command number won't match*/

                    commandNum = shift; 

                } else {

                    commandNum = i + 1; /* Number to print = array index + 1 */
                }
                
                printf("\n%d: ", commandNum);
                printf("%s", buffer[i]);
            
        }
        printf("\nEnter 'r' to run most recent command, 'r x' to run command starting with x.\n");

        /* Control flow to return to main for command prompt to reappear*/

    } else { /* If no signal, continue setup as normal */

        start = -1;
        if (length == 0)
            exit(0);            /* ^d was entered, end of user command stream */
        if (length < 0){
            perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
        }

        /* This for loop finds \n and replaces with \0 to turn whole command into string*/
        for (int i = 0; i < MAX_LINE; i++) {

            if (inputBuffer[i] == '\n'){

                inputBuffer[i] == '\0'; /* Null terminator is needed to copy whole command into buffer as string */
                break;
            }
        }

        if (bufferIndex == 10) { /* Buffer is full of commands, need to shift to add newest*/

            for (int i = 0; i < 8; i++){ /* Oldest command is removed, slot 9 made empty for new command */

                buffer[i] = buffer[i + 1]; 
            }
            bufferIndex = 9; /* Reassign to 9*/
            memcpy(buffer[bufferIndex], inputBuffer, strlen(inputBuffer) + 1); /* Copies command into buffer */

        } else { /* Array is not full of commands yet, add then increment */

            buffer[bufferIndex] = malloc(strlen(inputBuffer) + 1); /* Creates space in buffer to copy command */
            memcpy(buffer[bufferIndex], inputBuffer, strlen(inputBuffer) + 1); /* Copies command into buffer */
            bufferIndex ++;
        }

        /* examine every character in the inputBuffer */
        for (i = 0; i < length; i++) {
            switch (inputBuffer[i]){
            case ' ':
            case '\t' :               /* argument separators */
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1){
                    args[ct] = &inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            case '&':
                *background = 1;
                inputBuffer[i] = '\0';
                break;

            default :             /* some other character */
                if (start == -1)
                    start = i;
            }
        }
        args[ct] = NULL; /* just in case the input line was > 80 */
        numberOfCommands ++;
        
    }
    
}

int main(void)
{
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;             /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2+1];   /* 80-character command line has 40 args max */

    /* set up the signal handler */
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT; 
    sigaction(SIGINT, &handler, NULL);
    //strcpy(buffer,"Caught <ctrl><c>\n");

      
    char ch = ' ';

    for (int i = 0; i < 10; i++){
        buffer[i] = &ch;
    } 


    while (1){            /* Program terminates normally inside setup */
        background = 0;
        printf("COMMAND->");
        fflush(0);
        setup(inputBuffer, args, &background);       /* get next command */

        if (sig == 1) { /* If signal just happened, set sig to 0 and restart loop */

            sig = 0;
        } else { /* Continue as normal*/

            pid_t process = fork(); // forks a child process

            if (process < 0) { // checks if process creation failed

                printf("\nFork Failed!");
                exit(1); // terminates process
            }

            if (process == 0) { // child has process = 0, so only child will start executing this code

                //printf("\nAbout to run execvp()\n");
                printf("\nargs[0]: %c", args[0]);
                int valid = execvp(args[0], args); // calls execvp using the command and parameters and stores return value

                if (valid < 0) { //checks if execvp failed

                    printf("Invalid command!\n");
                    exit(1); //terminates child process if command is invalid
                }
            }

            // parent process continues since process != 0, and runs this code below
            if (background == 0) {

                //printf("\nEntered background == 0");
                process = wait(NULL); // causes parent to wait until child terminates
                //printf("\nwait(NULL) just ran!\n");
            }     
            // control flow returns to setup()

            /* Your code performs the following steps:
                (1) fork a child process using fork();
                (2) the child (foreground) process runs using execvp(); and
                (3) if background == 0, the parent waits.
            Otherwise, the control flow should go back to the setup() function to
            run the next command. (Hint: it's easier than you might think!) */

        }
    }
}
