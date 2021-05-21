# Commando
The goal of this project is to write a simple, quasi-command line shell called commando. The shell will be less functional in many ways from standard shells like bash (default on most Linux machines), but will have some properties that distinguish it such as the ability to recall output for any child process. Commando uses a variety of system calls together to accomplish its overall purpose. 

This project explores the following systems programming topics:
###### 1) Basic C Memory Discipline: 
A variety of strings and structs are allocated and de-allocated during execution which requires attention to detail and use of memory tools like Valgrind.
###### 2) fork() and exec(): 
Text entered that is not recognized as a built-in is treated as an command (external program) to be executed. This spawns a child process which executes the new program.
###### 3) Pipes, dup2(), read(): 
Rather than immediately print child output to the screen, child output is redirected into pipes and then retrieved on request by commando.
###### 4) wait() and waitpid(), blocking and nonblocking: 
Child processes usually take a while to finish so the shell will check on their status every so often

#### How to Run Code:
Be sure to include all files within same directory before running code. Open up a new command line shell (e.g. Terminal), navigate to the directory where files are saved, and type/enter the following:

$ make

$ ./commando

