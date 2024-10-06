## UID: 205838465

## Pipe Up

This program simulates the shell pipe (|) operator, allowing the output of one command to be used as the input for the next command.


## Building

To build the program, navigate to the directory containing the pipe.c file and run:
'make', which will compile the source code and create an executable named pipe.


## Running

To run the program, you can type the following:

'./pipe [command1] [command2] ... [commandN]'

Example: 
'./pipe ls wc'
This command would output the same as the command ls | wc

## Cleaning up

To remove all the files and the executable, use this command: 

'make clean'

This will delete the pipe.o object file and the pipe executable.
