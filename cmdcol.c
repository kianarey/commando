// cmdcol.c: Functions to deal with the cmdcolt struct

#include "commando.h"

void cmdcol_add(cmdcol_t *col, cmd_t *cmd)
/* Add the given cmd to the col structure. Update the cmd[] array and
  size field. Report an error if adding would cause size to exceed
  MAX_CMDS, the maximum number commands supported.
*/
{
  if(col->size < MAX_CMDS){ // check to make sure size is within bounds

    // Add the given cmd to the col structure.
    col->cmd[col->size] = cmd;

    // increment temp_size after given cmd is added to col struct
    col->size = col->size + 1; // Update size to the the updated size
  }
  else{
    perror("Size is larger than MAX_CMDS. Cannot be added. Exiting.");
    exit(1);
  }
}

void cmdcol_print(cmdcol_t *col)
/* Print all cmd elements in the given col structure.  The format of
  the table is

  JOB  #PID      STAT   STR_STAT OUTB COMMAND
  0    #17434       0    EXIT(0) 2239 ls -l -a -F
  1    #17435       0    EXIT(0) 3936 gcc --help
  2    #17436      -1        RUN   -1 sleep 2
  3    #17437       0    EXIT(0)  921 cat Makefile

  Widths of the fields and justification are as follows

  JOB  #PID      STAT   STR_STAT OUTB COMMAND
  1234 #12345678 1234 1234567890 1234 Remaining
  left  left    right      right rigt left
  int   int       int     string  int string

  The final field should be the contents of cmd->argv[] with a space
  between each element of the array.
*/
{
  // print labels on top
  printf("%-4s %-8s %4s %10s %4s %s\n", "JOB", "#PID", "STAT", "STR_STAT", "OUTB", "COMMAND");
  // use for loop to print row by row
  for(int i = 0; i < col->size; i++){
    printf("%-4d #%-8d %4d %10s %4d ", i, col->cmd[i]->pid, col->cmd[i]->status, col->cmd[i]->str_status, col->cmd[i]->output_size);

    // print the last string argv
    int j = 0, k = 0;
    while(col->cmd[i]->argv[j + k] != NULL){
      printf("%s ", col->cmd[i]->argv[j + k]);
      k++;
    }
    printf("\n"); // Enter new space, and start new print line
  }
}

void cmdcol_update_state(cmdcol_t *col, int nohang)
/* Update each cmd in col by calling cmd_update_state() which is also
  passed the block argument (either NOBLOCK or DOBLOCK)
*/
{
  for(int i = 0; i < col->size; i++){
    cmd_update_state(col->cmd[i], nohang); // Is this all I have to do?
  }

}

void cmdcol_freeall(cmdcol_t *col)
/* Call cmd_free() on all of the constituent cmd_t's.
*/
{
  for (int i = 0; i < col->size; i++){
    cmd_free(col->cmd[i]);
  }
}
