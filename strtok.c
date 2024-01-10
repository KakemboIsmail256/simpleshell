#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
size_t buffer_index = 0;

ssize_t custom_getline(char **lineptr, size_t *n, FILE *stream);
void tokenize_input(char *input, char *args[]);

void execute_command(char *args[]);

int main(void)
{
    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    while (1)
    {
        if (isatty(STDIN_FILENO))
            printf("myshell$ ");

        read = custom_getline(&input, &len, stdin);

        if (read == -1)
        {
            if (feof(stdin))
            {
                printf("\n");  // Handle "end of file" (Ctrl+D)
                break;
            }
            else
            {
                perror("custom_getline");
                exit(EXIT_FAILURE);
            }
        }

        // Remove the newline character from the input
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
            break;

        // Tokenize the input into arguments
        char *args[MAX_ARGS];
        tokenize_input(input, args);

        execute_command(args);

        if (!isatty(STDIN_FILENO))
            break;
    }

    free(input);
    exit(EXIT_SUCCESS);
}

ssize_t custom_getline(char **lineptr, size_t *n, FILE *stream)
{
    ssize_t bytes_read;
    size_t i = 0;

    while (1)
    {
        // If the buffer is empty, read more characters
        if (buffer_index == 0)
        {
            bytes_read = read(fileno(stream), buffer, BUFFER_SIZE);
            if (bytes_read == -1)
            {
                return -1;
            }
            else if (bytes_read == 0)
            {
                return i == 0 ? 0 : i;
            }
            buffer_index = bytes_read;
        }

        // Copy characters from the buffer to the lineptr
        while (i < *n - 1 && i < buffer_index)
        {
            (*lineptr)[i++] = buffer[buffer_index - i];
        }

        buffer_index -= i;

        // Check if we need to reallocate the lineptr
        if (i == *n - 1)
        {
            *n *= 2;
            *lineptr = realloc(*lineptr, *n);
            if (*lineptr == NULL)
            {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Check for newline character
        if (i > 0 && (*lineptr)[i - 1] == '\n')
        {
            (*lineptr)[i] = '\0';
            return i;
        }
    }
}

void tokenize_input(char *input, char *args[])
{
    size_t arg_count = 0;
    char *token = input;

    while (*input != '\0')
    {
        if (*input == ' ' || *input == '\t')
        {
            *input = '\0'; // Replace spaces or tabs with null terminators
            if (token != NULL)
            {
                args[arg_count++] = token;
                token = NULL;
            }
        }
        else if (token == NULL)
        {
            token = input;
        }

        ++input;
    }

    if (token != NULL)
    {
        args[arg_count++] = token;
    }

    args[arg_count] = NULL;
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
