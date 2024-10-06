#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>

void execute_command(char *command, int input_fd, int output_fd) {
    if (input_fd != STDIN_FILENO) {
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("dup2 input");
            exit(errno);
        }
        close(input_fd);
    }
    if (output_fd != STDOUT_FILENO) {
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("dup2 output");
            exit(errno);
        }
        close(output_fd);
    }
    execlp(command, command, (char *)NULL);
    perror("execlp");
    exit(127);  // Exit with a non-zero code indicating command not found
}

int pipe_args(int argc, char *argv[]) {
    int fd[2];
    int input_fd = STDIN_FILENO;
    int status;
    pid_t pid;

    for (int i = 1; i < argc - 1; ++i) {
        if (pipe(fd) == -1) {
            perror("pipe");
            return errno;
        }

        pid = fork();
        if (pid == -1) {
            perror("fork");
            return errno;
        }

        if (pid == 0) { // Child process
            close(fd[0]);
            execute_command(argv[i], input_fd, fd[1]);
        } else { // Parent process
            close(fd[1]);
            if (input_fd != STDIN_FILENO) {
                close(input_fd);
            }
            input_fd = fd[0];
        }
    }

    // Execute the last command
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return errno;
    }
    if (pid == 0) {
        execute_command(argv[argc - 1], input_fd, STDOUT_FILENO);
    }

    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }

    // Wait for all child processes
    while ((pid = wait(&status)) > 0) {
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            return WEXITSTATUS(status);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        errno = EINVAL;
        perror("Requires at least 1 argument");
        return errno;
    }

    return pipe_args(argc, argv);
}
