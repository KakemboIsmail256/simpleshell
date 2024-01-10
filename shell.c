#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024

void execute_command(char *command);

int main(void)
{
    char input[MAX_INPUT];
    size_t len = 0;
    ssize_t read;

    while (1)
    {
        if (isatty(STDIN_FILENO))
            printf("($) ");
        
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

        execute_command(input);

        if (!isatty(STDIN_FILENO))
            break;
    }

    free(input);
    exit(EXIT_SUCCESS);
}

void execute_command(char *command)
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
        // This is the child process
        char *args[] = {command, NULL};
        execve(command, args, NULL);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else
    {
        // This is the parent process
        waitpid(child_pid, &status, 0);
    }
}
