/*
  commando.c: main() function for commando interactive shell, which loops over input provided by user either interactively or via standard input as is done in the automated tests. After setup, the program   executes an infinite loop until no more input is available.

  Print the @> prompt and parse input
  Determine what action to take: built-in or start a job
  Check for updates to the state of jobs and print alerts for changes

  Some input may cause new cmd_t instances to be allocated for child processes. These are added into a cmdcol_t instance for tracking.
*/

#include "commando.h"

int main(int argc, char *argv[]){
  setvbuf(stdout, NULL, _IONBF, 0); // Turn off output buffering
  // check and set environment variables via the standard getenv() and setenv() fumctions

  char *echo = getenv("COMMANDO_ECHO"); // returns a pointer to a value associated with name, NULL if not found
  char *echo_str = "--echo";

  // built-ins
  char *commands[] = {"help", // 0
  "exit", // 1
  "list", // 2
  "pause", // 3
  "output-for", // 4
  "output-all", // 5
  "wait-for", // 6
  "wait-all"}; // 7

  char buffer[MAX_LINE]; // Fixed character buffer used to read in input
  char *tokens[ARG_MAX+1];
  int ntoks;
  char *input = NULL;

  // It makes better sense to put this in the while loop, because every time it loops it's getting a new cmd until exit, but can't free if not outside of while loop

  cmdcol_t *new_cmdcol = malloc(sizeof(cmdcol_t)); // There is only one of this!!
  new_cmdcol->size = 0; //initialize size to 0

  while(1){
    printf("@> "); // print the @> prompt

    // need to ensure buffer isn't overflowed
    if(sizeof(buffer) <= MAX_LINE){
      input = fgets(buffer, MAX_LINE, stdin); // get input string in C programming
    }
    // if no input remains, print End of input and break out of loop
    if(input == NULL){
      printf("\nEnd of input");
      break;
    }

    // Echo (print) given input if echoing is enabled. Three total cases
    /*
    |````````|````````|
    | unset  | export |
    | --echo | --echo |
    | enable | enable |
    |````````|````````|
    | unset  | export |
    |  NULL  |  NULL  |
    | disable| enable |
    ```````````````````
    The only time when echo should not print is when COMMANDO_ECHO is unset and there is no argv[1] argument.
    */
    if(argv[0] && !argv[1]){ // just ./commando
      // if echo is set via export AND argv[0] matches ./commando
      if(echo != NULL && strncmp(argv[0], "./commando", strlen("./commando")) == 0){
        int i = 0;
        while(input[i] != '\0'){
          printf("%c", input[i]);
          i++;
        }
      }
    }
    if(argv[0] && argv[1]){ // ./commando --echo
      // if echo is unset AND argv[1] matches --echo
      if(echo == NULL && strncmp(argv[1], echo_str, strlen(echo_str)) == 0){
        int i = 0;
        while(input[i] != '\0'){
          printf("%c", input[i]);
          i++;
        }
      }
      // if echo is set AND argv[1] matches --echo
      if(echo != NULL && strncmp(argv[1], echo_str, strlen(echo_str)) == 0){
        int i = 0;
        while(input[i] != '\0'){
          printf("%c", input[i]);
          i++;
        }
      }
    }

    // Parse input using parse_into_tokens from util.c to produce argv[]. It is for sure null terminated. See util.c
    parse_into_tokens(input, tokens, &ntoks);

    if(ntoks != 0){
      // built-ins:
      // help cmd
      if(strncmp(tokens[0], commands[0], strlen(commands[0])) == 0){ // if the 0th token is help; strncmp returns 0 if identical
        printf("COMMANDO COMMANDS\n");
        printf("help              : show this message\n");
        printf("exit              : exit the program\n");
        printf("list              : list all jobs that have been started giving information on each\n");
        printf("pause nanos secs  : pause for the given number of nanseconds and seconds\n");
        printf("output-for int    : print the output for given job number\n");
        printf("output-all        : print output for all jobs\n");
        printf("wait-for int      : wait until the given job number finishes\n");
        printf("wait-all          : wait for all jobs to finish\n");
        printf("command arg1 ...  : non-built-in is run as a job\n");
      }

      // exit cmd
      else if(strncmp(tokens[0], commands[1], strlen(commands[1])) == 0){
        break;
      }

      // list cmd
      else if(strncmp(tokens[0], commands[2], strlen(commands[2])) == 0){
        //cmdcol_add(new_cmdcol, new_cmd);
        cmdcol_print(new_cmdcol); // Is this right?
      }

      // pause nano secs cmd
      else if(strncmp(tokens[0], commands[3], strlen(commands[3])) == 0){
        long nano;
        int secs;
        if(!tokens[1] && !tokens[2]){
          nano = 0;
          secs = 0;
        }
        if(tokens[1] && !tokens[2]){ // If user didn't input anything for secs
          nano = atoi(tokens[1]); // atoi convert string to int
          secs = 0;
        }
        if(tokens[1] && tokens[2]){ // If user input for both
          nano = atoi(tokens[1]);
          secs = atoi(tokens[2]);
        }
        pause_for(nano, secs);
      }

      // output-for int cmd
      else if(strncmp(tokens[0], commands[4], strlen(commands[4])) == 0){
        int job_num = atoi(tokens[1]); // this is the job number
        // print this job
        printf("@<<< Output for %s[#%d] (%d bytes):\n", new_cmdcol->cmd[job_num]->name, new_cmdcol->cmd[job_num]->pid, new_cmdcol->cmd[job_num]->output_size);
        printf("----------------------------------------\n");
        //cmd_fetch_output(new_cmdcol->cmd[job_num]); // Do I need this?
        cmd_print_output(new_cmdcol->cmd[job_num]);
        printf("----------------------------------------\n");
      }

      // output-all cmd
      else if(strncmp(tokens[0], commands[5], strlen(commands[5])) == 0){
        // loop through and print all output
        for (int i = 0; i < new_cmdcol->size; i++){
          printf("@<<< Output for %s[#%d] (%d bytes):\n", new_cmdcol->cmd[i]->name, new_cmdcol->cmd[i]->pid, new_cmdcol->cmd[i]->output_size);
          printf("----------------------------------------\n");
          //cmd_fetch_output(new_cmdcol->cmd[job_num]); // Do I need this?
          cmd_print_output(new_cmdcol->cmd[i]);
          printf("----------------------------------------\n");
        }
      }

      // wait-for int cmd
      else if(strncmp(tokens[0], commands[6], strlen(commands[6])) == 0){
        int wait = atoi(tokens[1]);

        // The wait-for int command translates to a call to cmd_update_state() with the DOBLOCK option.
        if(new_cmdcol->size != 0 && new_cmdcol->cmd[wait]){ // check to make sure this job actually exists
          cmd_update_state(new_cmdcol->cmd[wait], DOBLOCK);
        }
      }

      // wait-all cmd
      else if(strncmp(tokens[0], commands[7], strlen(commands[7])) == 0){

        // wait for all commands
        for(int i = 0; i < new_cmdcol->size; i++){
          cmd_update_state(new_cmdcol->cmd[i], DOBLOCK);
        }
      }

      // command argl
      else{
        // 0th token do not match above cmds. Create a new cmd_t instance where the tokens are the argv[] for it and start running it.
        //cmd_t *cmd_argl = malloc(sizeof(cmd_t);
        cmd_t *new_cmd = cmd_new(tokens);
        //new_cmd = cmd_new(tokens); THIS SON OF A GUN CAUSED ME SO MUCH HEADACHE
        //Debugging
        /*
        printf("Printing contents within new_cmd which is a pointer to cmd_t\n");
        printf("char name[NAME_MAX+1]; is: %-10s\n", new_cmd->name);
        printf("\n");

        printf("Printing char *argv[ARG_MAX+1];\n");
        int d = 0;
        while(new_cmd->argv[d] != NULL){
          printf("\tchar *argv[%d]; is: %-10s\n", d, new_cmd->argv[d]);
          d++;
        }
        printf("pid_t pid; is: %-10d \n", new_cmd->pid);
        printf("\n");

        printf("Printing int out_pipe[];\n");
        printf("int out_pipe[0]; is: %-10d\n", new_cmd->out_pipe[0]);
        printf("int out_pipe[1]; is: %-10d\n", new_cmd->out_pipe[1]);

        printf("int finished; is: %-10d\n", new_cmd->finished);
        printf("int status; is: %-10d\n", new_cmd->status);
        printf("char str_status[STATUS_LEN+1]; is: %-10s\n", new_cmd->str_status);
        printf("int output_size; is: %-10d\n", new_cmd->output_size);
        printf("\n");
        */
        cmdcol_add(new_cmdcol, new_cmd); // add this to cmdcol_t

        /* Debugging cmdcol_add
        printf("Printing all cmds in cmdcol_t struct\n");
        int i;
        for(i = 0; i < new_cmdcol->size; i++){
          printf("new_cmdcol->cmd[%d]->name is: %-10s\n", i, new_cmdcol->cmd[i]->name);

          int k = 0;
          while(new_cmdcol->cmd[i]->argv[k] != NULL){
            printf("new_cmdcol->cmd[%d]->argv[%d] is: %-10s\n", i, k, new_cmdcol->cmd[i]->argv[k]);
            k++;
          }
          printf("\n");
        }
        printf("\n");

        printf("Printing size in cmdcol_t struct\n");
        printf("int size; is: %-10d\n", new_cmdcol->size);
        printf("\n");

        */
        cmd_start(new_cmd); // start running
        //printf("2. Child PID is %d: \n", new_cmd->pid);

      }
    }
    // At the end of each iteration of the main loop of commando, each job should be checked for updates to its status. cmdcol_update_state() is a good idea to update everything. This call should not block.
    cmdcol_update_state(new_cmdcol, NOBLOCK);

  }
  // free all dynamically allocated memory/ptrs

  cmdcol_freeall(new_cmdcol); // Will this do the trick?
  free(new_cmdcol);

  return 0;
}
