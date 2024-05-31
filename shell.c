#include <stdio.h>
#include <stdlib.h>
#define LSH_RL_BUFSIZE 1024 //temperorily makes a buffer to prevent overhead
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char *lsh_read_line(void) //this function reads the line and puts it into memory
{
    int bufsize = LSH_RL_BUFSIZE; //defining buffsize
    int position = 0; //current position
    char *buffer = malloc(sizeof(char) * bufsize); //stores memory for buffer by allocating memory with sizeof(char) and bufsize
    int c; //current char allocated

    if(!buffer){ //exits buffer if memory allocated incorrectly
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        c = getchar(); //this reads the character from the stream 

        if(c == EOF || c == '\n'){ //this checks if the c is an EOF or newline due to line oriented programming
            buffer[position] = '\0'; //ends the buffer and returns it
            return buffer; //returns memory
        } else {
            buffer[position] = c; //adds char to buffer memory
        }
        position++; //updates position in memory
    }

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) { //checks if position is greater
      bufsize += LSH_RL_BUFSIZE; //adds the buffersize back
      buffer = realloc(buffer, bufsize); //changes size to bufsize by pointing to buffer
      
      if (!buffer) { //if allocation memory error occurs
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }


    }

}

//parsing the line double pointers are strings because pointers to arrays are strings so pointer to pointers are strings
char **lsh_split_line(char *line) //takes in a pointer to the characters and outputs the strings
{
    int bufsize = LSH_TOK_BUFSIZE; //buffer size
    int position = 0; //current position
    char **tokens = malloc(bufsize * sizeof(char*)); //declares tokens as a pointer to pointers to char making it an array of strings
    //allocates memory based on bufsize and sizeof char
    char *token; //string at time

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE); 
    }

    token = strtok(line, LSH_TOK_DELIM); //splits line variable based on delim

    while (token != NULL) { //while the token is not null
        tokens[position] = token; //sets array to that
        position++; //updates position

        if(position >= bufsize){ //reallocates memory
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) { //memory allocation error
            fprintf(stderr, "lsh: allocation error\n");
            exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM); //seperates token by NULL tells strtok to use the internal pointer it updated during the previous call, rather than starting over with a new string.
    }
    tokens[position] = NULL; //final position to null
    return tokens; //returns memory parsed

}

//launch file 
int lsh_launch(char **args){ //takes in cmd string
    pid_t pid; //process ids for the fork 
    pid_t wpid; //process ids waitpid() while another process finishes 

    int status; //current exit status

    pid = fork();
  if (pid == 0) {
    // Child process
    printf("This is the child process. PID: %d\n", getpid());
    if (execvp(args[0], args) == -1) { //The child process is a copy of the parent process but has a different PID.
      perror("lsh"); //prints error to stream
      exit(EXIT_FAILURE);
    }
    // the child can also replace its code with a new program as shown below 
    // execvp("new_program", args);
  } else if (pid < 0) {
    // Error forking
    perror("fork failed");
    exit(EXIT_FAILURE);
  } else {
    // Parent process
    printf("This is the parent process. PID: %d, Child PID: %d\n", getpid(), pid);
    do { // The parent process continues executing from the point where fork() was called.
      wpid = waitpid(pid, &status, WUNTRACED); 
      //waitpid() is a system call used to wait for state changes in a child process.
      //
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("Child stopped by signal %d\n", WSTOPSIG(status));
        }
  }

  return 1;
}



//Read: Read the command from standard input.
//Parse: Separate the command string into a program and arguments.
//Execute: Run the parsed command.

void lsh_loop(void){ //exectuion
    char *line; //stores the line form the user
    char **args; //by being a pointer to a pointer this represents the pointer to a string and they point to chars
    int status; //current status exits when non zero

   do { 
    printf("> "); //ready to accept input
    line = lsh_read_line(); //reads the line
    args = lsh_split_line(line); //splits input on spaces
    status = lsh_execute(args); //executes status based on args

    free(line); //free memory from line and args 
    free(args);
    } while(status);
}





int main(int argc, char **argv){

    // Load config files, if any.


    // Run command loop.
    lsh_loop();

    // Perform any shutdown/cleanup.

    return 0;
}