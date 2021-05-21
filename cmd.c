/*
  cmd.c: functions related the cmd_t struct abstracting a command. Most functions maninpulate cmd_t structs.
*/

#include "commando.h"

cmd_t *cmd_new(char *argv[]) // takes in an array of string arguments
/*
  Allocates a new cmd_t with the given argv[] array. Makes string
  copies of each of the strings contained within argv[] using
  strdup() as they likely come from a source that will be
  altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
  field to be the argv[0]. Sets finished to 0 (not finished yet). Set
  str_status to be "INIT" using snprintf(). Initializes the remaining
  fields to obvious default values such as -1s, and NULLs.
*/
{
  // First step is to use malloc() to allocate a hunk of memory (cmd_t)
  cmd_t *new = malloc(sizeof(cmd_t)); // new is a pointer to cmd_t struct

  int i = 0; // i marks current position
  while(argv[i] != NULL){
    new->argv[i] = strdup(argv[i]); // Makes string copies of each and stores them into argv[], which is an array of string arguments
    //printf("new->argv[%d] is: %s\n", i, new->argv[i]); // debugger
    i++; // Increment i
  }
  new->argv[i] = NULL; // Ensures that cmd->argv[] is null-terminated
  //printf("new->argv[%d] is: %s\n", i, new->argv[i]);

  // Sets the name field to be the argument at argv[0]. What this does is it essentially copies the entire string at argv[0] and stores it in char name[NAME_MAX+1].
  strcpy(new->name, new->argv[0]);

  new->finished = 0; // Sets finished to 0 (not finished yet)

  /* Personal note
    snprintf(char *s, size_t n, const char * format, ...)
    s = point to a buffer where the results C-string is stored. The buffer should have at least n characters

    n = max number of bytes to be used in the buffer
    The generated string has a lengthof at most n-1, leaving space for the additional terminating null character

    format = C string that contains a format string that follows the same specifications as format in printf

    ... = Depending on the format string, the function may expect a sequence of additional arguments, each containing a value to be used to replace a format specifier in the format string There should be at least as many of these arguments as the number of values specified in the format specifiers. Additional arguments are ignored by the function

    Return value = the number of characters that would have been written if n had been sufficiently large, not counting the terminating null character.

    Example:
    char buffer [100];
    in cx;

    cx = snprintf(buffer, 100, "The half of %d is %d", 60, 60/2);

    if (cx >= 0 && cx < 100){
      snprintf(buffer + cx, 100 - cx, ", and a half of that is %d.", 60/2/2);
    }

    puts(buffer); // prints onto screen, easier implementation
    return 0;
  */

  //Set str_status to be "INIT" using snprintf().
  snprintf(new->str_status, STATUS_LEN+1, "INIT"); //

  // Initializes the remainining fields to obvious default values such as -1s, and NULLs.
  new->pid = -1;
  new->out_pipe[0] = -1;
  new->out_pipe[1] = -1;
  new->status = -1;
  new->output = NULL;
  new->output_size = -1;

  return new;
}

void cmd_free(cmd_t *cmd)
/*
  Deallocates a cmd structure. Deallocates the strings in the argv[]
  array. Also deallocats the output buffer if it is not
  NULL. Finally, deallocates cmd itself.
*/
{
  int i = 0;
  while(cmd->argv[i] != NULL){ // While the argument is not NULL
    free(cmd->argv[i]); // Deallocates the strings in the argv[] array
    i++;
  }
  free(cmd->argv[i]); // Finally free the last string NULL

  if(cmd->output != NULL){
    free(cmd->output);
  }
  free(cmd); // Finally deallocates cmd itself.
}

/* Profesor said to ignore
void cmd_set_stdin(cmd_t *cmd, char *input_file){

}
*/

void cmd_start(cmd_t *cmd)
/*
  Forks a process and starts executes command in cmd in the process.
  Changes the str_status field to "RUN" using snprintf(). Creates a
  pipe for out_pipe to capture standard output. In the parent
  process, ensures that the pid field is set to the child PID. In the
  child process, directs standard output to the pipe using the dup2()
  command. For both parent and child, ensures that unused file
  descriptors for the pipe are closed (write in the parent, read in
  the child).
*/
{
    // Create a pipe associated with the cmd->out_pipe field
    // This way the parent and child has access to work with pipe
    pipe(cmd->out_pipe);

    // Ensure that cmd->str_status is changes to RUN, use snprintf()
    snprintf(cmd->str_status, STATUS_LEN+1, "RUN");

    // Fork a new process and capture its pid in the cmd->pid field
    // Can I do this? cmd->pid = fork();
    pid_t child = fork();
    if(child < 0){  // check if fork failed
      perror("Failed to fork"); // report errors if forking failed
      exit(1);
    }

    // Child process
    if(child == 0) // It's a child
    {
      //printf("I am the child\n"); // debugger
      // The child process will need to use dup2() to alter its standard output to write instead to the write to cmd->out_pipe[PRW]
      dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO);
      close(cmd->out_pipe[PREAD]); // child closes the read end of pipe

      // execvp format
      // char *new_argv[] = {"ls", "-l", NULL};
      // char command = "ls";
      // The child should call execvp() with the name of the command and argv[] array stored in the passed cmd. This should launch a new program with output that is directed into the pipe set up above

      execvp(cmd->name, cmd->argv); // Am I doing this right? Prof says yes.
    }
    else{ // Parent process

      //printf("I am the parent of child #%d\n", child); // debugger
      cmd->pid = child;
      //printf("I stored child's number in pid as #%d\n", cmd->pid);
      close(cmd->out_pipe[PWRITE]); // Parent closes the write end of pipe
    }

}

void cmd_update_state(cmd_t *cmd, int block)
/*
  If the finished flag is 1, does nothing. Otherwise, updates the
  state of cmd.  Uses waitpid() and the pid field of command to wait
  selectively for the given process. Passes block (one of DOBLOCK or
  NOBLOCK) to waitpid() to cause either non-blocking or blocking
  waits.  Uses the macro WIFEXITED to check the returned status for
  whether the command has exited. If so, sets the finished field to 1
  and sets the cmd->status field to the exit status of the cmd using
  the WEXITSTATUS macro. Calls cmd_fetch_output() to fill up the
  output buffer for later printing.

  When a command finishes (the first time), prints a status update
  message of the form

  @!!! ls[#17331]: EXIT(0)

  which includes the command name, PID, and exit status.
*/
{
  if(cmd->finished == 1){
    // do nothing
    return; // return
  }
  // update the state of cmd
  int status;
  int retcode = waitpid(cmd->pid, &status, block); // Get return value
  // Returned     Means
  // child_pid    status of child that changed or exited
  // 0            there is no status change for child / none exited
  // -1           an error
  /*
  If a state change has occurred, it can be dissected using a series of macros in the manual entry for wait() and waitpid(). The most important of these is the WIFEXITED(status) macro which is called on a status integer passed to waitpid().
  */

  if(retcode == 0){
      // there is no status change for child. Return.
      return;
  }
  else{ // status changed occurred
    if(WIFEXITED(status)){  // Determine if child actually exited, nonzero if exited.
      int retval = WEXITSTATUS(status);// Get return value of program, 0-255; nonzero exit codes usually inidicate failure.

      cmd->status = retval; // sets the cmd->status field to the exit status of the cmd
      snprintf(cmd->str_status, STATUS_LEN + 1, "EXIT(%d)", retval); // change cmd->str_status to EXIT(num) when the process finishes
      cmd->finished = 1; // set to finished
      cmd_fetch_output(cmd); // Calls cmd_fetch_output() to fill up the output buffer for later printing
      printf("@!!! %s[#%d]: %s\n", cmd->name, cmd->pid, cmd->str_status); // print message, only once per change/exit
      return; // done and return
    }
  }
}

char *read_all(int fd, int *nread)
/*
  Reads all input from the open file descriptor fd. Stores the
  results in a dynamically allocated buffer which may need to grow as
  more data is read. Uses an efficient growth scheme such as
  doubling the size of the buffer when additional space is
  needed. Uses realloc() for resizing.  When no data is left in fd,
  sets the integer pointed to by nread to the number of bytes read
  and return a pointer to the allocated buffer. Ensures the return
  string is null-terminated. Does not call close() on the fd as this
  is done elsewhere.
*/
{
  // See lab 3 and append_all.c file

  // Allocate some initial memory in a buffer to read() into from the file descriptor. For each read() call, limit the number of bytes read so that this buffer is not overflowed.
  int max_size = 1024; // start with this size prof says
  int cur_pos = 0; // set beginning position
  char *buffer = malloc(max_size*sizeof(char)); // dynamically allocate some memory to buffer

  // format for read(): read(int_fd, buffer, SIZE);
  // read() reads upt to SIZE from fd, stores bytes in buffer and returns number of bytes read, -1 for error

  while(1){ // while true
    if(cur_pos >= max_size){ // if cur_pos is greater than max_size
      max_size *= 2; // double max_size, 1024 to 2048 and so on
      buffer = realloc(buffer, max_size); // allocate more memory

      if(buffer == NULL){ // check for failure
        perror("Could not expand input buffer; Exiting.\n");
        exit(1);
      }
    }

    int max_read = max_size - cur_pos; // calculate max read
    int bytes_read = read(fd, buffer + cur_pos, max_read);

    cur_pos += bytes_read; // successful read, advance input buffer position

    if(bytes_read == 0){ // 0 bytes read indicates end of file/input
      //printf("End of input\n"); // debugger

      buffer = realloc(buffer, max_size + 1); // give one more space for null terminating character
      *nread = cur_pos; // cur_pos marks total bytes read out of max_size
      //printf("total read is %d\n", cur_pos);
      // read() function does NOT null terminate!!!

      buffer[cur_pos] = '\0'; // Ensures that buffer is null-terminated. Can I do this?

      //free(buffer); // Free allocated buffer?
      return buffer;
    }
    else if (bytes_read == -1){
      perror("Read failed");
      exit(1); // bail out, friends
    }
  }
}

void cmd_fetch_output(cmd_t *cmd)
/* If cmd->finished is zero, prints an error message with the format

  ls[#12341] not finished yet

  Otherwise retrieves output from the cmd->out_pipe and fills
  cmd->output setting cmd->output_size to number of bytes in
  output. Makes use of read_all() to efficiently capture
  output. Closes the pipe associated with the command after reading
  all input.
*/
{
    if(cmd->finished == 0){ // cmd is not done
      printf("%s[#%d] not finished yet", cmd->name, cmd->pid);
      return; // take no further action.
    }
    else{ // cmd is finished
      // retrieves output from the cmd->out_pipe[PREAD] and fills the cmd->output setting cmd->output_size to number of bytes in output.

      int nread = cmd->output_size;
      cmd->output = read_all(cmd->out_pipe[PREAD], &nread);

      //printf("nread after read_all returned is %d \n", nread); // debugger
      cmd->output_size = nread;
      close(cmd->out_pipe[PREAD]); // make sure to close the pipe
    }
}

void cmd_print_output(cmd_t *cmd)
/*
  Prints the output of the cmd contained in the output field if it is
  non-null. Prints the error message

  ls[#17251] : output not ready

  if output is NULL. The message includes the command name and PID.
*/
{
  if(cmd->output != NULL){
    // prints the output of the cmd
    // Use a call to write() to put data on the screen. As write() uses file descriptors, make sure to pass STDOUT_FILENO along with the buffer to write and the number of bytes to write
    write(STDOUT_FILENO, cmd->output, cmd->output_size);
  }
  else{ // prints the error message
    printf("%s[#%d] : output not ready\n", cmd->name ,cmd->pid);

  }
}
