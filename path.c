#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

void execute_command(char *args[]);

int main(void)
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    size_t len = 0;
    ssize_t read;

    while (1)
    {
        if (isatty(STDIN_FILENO))
            printf("myshell$ ");

        read = getline(&input, &len, stdin);

        if (read == -1)
        {
            if (feof(stdin))
            {
                printf("\n");  // Handle "end of file" (Ctrl+D)
                break;
            }
            else
            {
                perror("getline");
                exit(EXIT_FAILURE);
            }
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

        execute_command(args);

        if (!isatty(STDIN_FILENO))
            break;
    }

    free(input);
    exit(EXIT_SUCCESS);
}

void execute_command(char *args[])
{
    pid_t child_pid;
    int status;

    // Check if the command exists in the PATH
    char *path = getenv("PATH");
    char *path_copy = strdup(path);

    char *token = strtok(path_copy, ":");
    while (token != NULL)
    {
        // Construct the full path of the command
        char command_path[MAX_INPUT];
        snprintf(command_path, sizeof(command_path), "%s/%s", token, args[0]);

        // Attempt to execute the command
        if (access(command_path, X_OK) == 0)
        {
            child_pid = fork();

            if (child_pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (child_pid == 0)
            {
                // This is the child process
                execv(command_path, args);

                // If execv fails, print an error message
                perror("execv");
                exit(EXIT_FAILURE);
            }
            else
            {
                // This is the parent process
                waitpid(child_pid, &status, 0);
                break;  // Command executed, exit the loop
            }
        }

        token = strtok(NULL, ":");
    }

    // Free the allocated memory
    free(path_copy);
}
