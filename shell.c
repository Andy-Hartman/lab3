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
int mostRecent = 0; /* Index of most recent command saved in buffer */
int numberOfCommands = 0; /* Total number of commands entered by the user */
int sig = 0; /* Int used to track status of signal, if 1, signal was just triggered */
int validCommand = 0; /* Int used to track validity of command (0 = invalid, 1 = valid)*/
int found = 0; /* Int used to track status of command starting with char x for 'r x' input*/
char inputBufferCopy[MAX_LINE];



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

    sig = 1; /* Sets sig to 1, changing program control flow */
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

        if (numberOfCommands >= 10) { /*Print 10 most recent commands*/

            int commandNum = numberOfCommands - 9;

            for (int i = 0; i < 10; i++) {

                printf("\n%d: ", commandNum);
                printf("%s", buffer[i]);
                commandNum ++;
            }
        } else { /* Less than 10 commands*/

            for (int i = 0; i < numberOfCommands; i++) {

                printf("\n%d: ", i + 1);
                printf("%s", buffer[i]);
            }
        }
        printf("\n");

        /* Control flow to return to main for command prompt to reappear*/

    } else { /* If no signal, continue setup as normal */

        start = -1;
        if (length == 0)
            exit(0);            /* ^d was entered, end of user command stream */
        if (length < 0){
            perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
        }

        /* Checks if user entered r or r + x, if so, finds command in buffer and swaps it into input buffer */
        
        if (inputBuffer[0] == 'r') { /* User entered r as first character */

            
            found = 0; /* Reset found to 0 */

            if (inputBuffer[1] == '\n'){ /* User entered just r, run most recent command */
                found = 1;
                
                memcpy(inputBuffer, buffer[mostRecent], strlen(buffer[mostRecent]) + 1); /* Assign most recent command to inputBuffer*/

            } else if (inputBuffer[1] = ' ') { /* User specified command to run based on first character */

                char firstLetter = inputBuffer[2]; /* First letter of command */

                /* Iterate through buffer, checking each command's first letter*/

                for (int i = 9; i >= 0; i--) {

                    if (buffer[i][0] == firstLetter) { /* Found command */

                        /* Feed command to inputBuffer*/
                        memcpy(inputBuffer, buffer[i], strlen(buffer[i]) + 1); 
                        found = 1;
                        i = -1; /* Break from for-loop*/
                    }
                }
            }
        } else { /* User did not enter r*/

            found = 1; /* Sets found to 1 so regular setup code runs */
        }

        if (found == 1) { /* Run setup because there is a command to run in inputBuffer*/

            /* Memcpy inputBuffer into inputBufferCopy to add to buffer later if command is valid */

            for (int i = 0; i < MAX_LINE; i++){ /* Makes sure there is null terminator*/

                if (inputBuffer[i] == '\n') {

                    inputBuffer[i] = '\0';
                    break;
                }
            }

            memcpy(inputBufferCopy, inputBuffer, strlen(inputBuffer) + 1);
            length = strlen(inputBuffer) + 1; /* Adjust length */

            for (int i = 0; i < MAX_LINE; i++){ /* Swaps null terminator for newline*/

                if (inputBuffer[i] == '\0') {

                    inputBuffer[i] = '\n';
                    break;
                }
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
        }
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
      
    char ch = '\0';

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
        } else if (found == 0){ /* Command was not found*/

            printf("Error. No command found.\n");
        } else { /* Continue as normal*/

            pid_t process = fork(); // forks a child process

            if (process < 0) { // checks if process creation failed

                printf("\nFork Failed!");
                exit(1); // terminates process
            }

            if (process == 0) { // child has process = 0, so only child will start executing this code

                validCommand = 0; /* Reset validity */
                validCommand = execvp(args[0], args); // calls execvp using the command and parameters and stores return value

                if (validCommand < 0) { //checks if execvp failed

                    printf("Invalid command!\n");
                    exit(1); //terminates child process if command is invalid
                }                 
            }

            // parent process continues since process != 0, and runs this code below
            if (background == 0) {

                process = wait(NULL); // causes parent to wait until child terminates
            }     

            if (inputBufferCopy[0] == '\0') { /* Prevents null entries from being added to buffer*/

                    validCommand = -1; 
            }

            if (validCommand >= 0) { /* Command was valid, add to buffer */

                numberOfCommands ++;

                if (bufferIndex == 10) { /* Buffer is full of commands, need to shift to add newest*/

                    for (int i = 0; i <= 8; i++){ /* Oldest command is removed, slot 9 made empty for new command */

                        free(buffer[i]);
                        buffer[i] = malloc(strlen(buffer[i + 1]) + 1);
                        memcpy(buffer[i], buffer[i + 1], strlen(buffer[i + 1]) + 1);
                    }
                    bufferIndex = 9; /* Reassign to 9*/
                    free(buffer[9]);
                    buffer[9] = malloc(strlen(inputBufferCopy) + 1);
                    memcpy(buffer[9], inputBufferCopy, strlen(inputBufferCopy) + 1); /* Copies command into buffer */

                } else { /* Array is not full of commands yet, add then increment */
                            
                    buffer[bufferIndex] = malloc(strlen(inputBufferCopy) + 1); /* Creates space in buffer to copy command */
                    memcpy(buffer[bufferIndex], inputBufferCopy, strlen(inputBufferCopy) + 1); /* Copies command into buffer */
                    mostRecent = bufferIndex; /* Most recent command = one just added @bufferIndex */
                    bufferIndex ++;
                }
            }
        }
    }
}
