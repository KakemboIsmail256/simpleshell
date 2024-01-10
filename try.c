#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

void execute_command(char *args[], int input_fd, int output_fd);

int main(void)
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    size_t len = 0;
    ssize_t read;

    while (1)
    {
        printf("myshell$ ");
        read = getline(&input, &len, stdin);

        if (read == -1)
        {
            perror("getline");
            exit(EXIT_FAILURE);
        }

        // Remove the newline character from the input
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        // Tokenize the input into arguments
        size_t arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARGS - 1)
        {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;

        execute_command(args, STDIN_FILENO, STDOUT_FILENO);
    }

    free(input);
    exit(EXIT_SUCCESS);
}

void execute_command(char *args[], int input_fd, int output_fd)
{
    pid_t child_pid;
    int status;

    child_pid = fork();

    if (child_pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0)
    {
        // Child process
        if (input_fd != STDIN_FILENO)
        {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO)
        {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        waitpid(child_pid, &status, 0);
    }
}
