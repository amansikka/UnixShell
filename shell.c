#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>


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
            printf("Child exited with status %d\n", WEXITSTATUS(status)); //if exited 
        } else if (WIFSIGNALED(status)) {
            printf("Child terminated by signal %d\n", WTERMSIG(status)); //if terminated 
        } else if (WIFSTOPPED(status)) {
            printf("Child stopped by signal %d\n", WSTOPSIG(status)); //if stopped
        }
  }

  return 1;
}

int lsh_cd(char **args); //changing of a directory taking in args
int lsh_help(char **args); //changing for help with specific arguments
int lsh_exit(char **args); //exiting shell with specific exit statuses

char *builtin_str[] = { //possible commands
  "help",
  "exit",
  "cd"
}; 

int (*builtin_func[]) (char **) = { //array of function pointers 
  &lsh_cd, //cd dir
  &lsh_help, //help cmd
  &lsh_exit //exit cmd
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *); //returns size of the commands a based on bytes
}

int lsh_cd(char **args)
{
  if (args[1] == NULL) { //checks if a dir was specified
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) { //attempts to change with chdir()
      perror("lsh"); //if so fails error message
    }
  }
  return 1;
}

int lsh_help(char **args)
{
  int i;
  printf("Aman Sikka's custom shell\n");
  printf("Type program names and args then hit enter\n");
  printf("Everything else is built in :\n");

  for (i = 0; i < lsh_num_builtins(); i++) { //loops through every built in and prints
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int lsh_exit(char **args)
{
  return 0; //just retuns 0
}





int lsh_execute(char **args)
{

  if (args[0] == NULL) { 
    // An empty command was entered.
    return 1;
  }

  for (int i = 0; i < lsh_num_builtins(); i++) { //loops through
    if (strcmp(args[0], builtin_str[i]) == 0) { //means user has entered a correct shell cmd
      return (*builtin_func[i])(args); //returns functor with args
    }
  }

  return lsh_launch(args); //proceeds to launch with args
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
    free(args); //free memory for args
    } while(status);
}





int main(int argc, char **argv){

    // Load config files, if any.


    // Run command loop.
    lsh_loop();

    // Perform any shutdown/cleanup.

    return 0;
}

