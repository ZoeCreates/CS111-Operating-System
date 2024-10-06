# You Spin Me Round Robin

This project implements a round-robin CPU scheduling algorithm simulation in C.

## Building

To build the program:

```shell
gcc -o round_robin round_robin.c
```

## Running

To run the program, use the following command.

```shell
./round_robin <input_file> <time_quantum>
```
The program will output the average waiting time and average response time for the processes in the following format:

```shell
Average waiting time: X.XX
Average response time: Y.YY

```

## Cleaning up

To remove the round_robin executable file, run the following: 

```shell
rm round_robin
```


