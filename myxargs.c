#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h> // Include for basename function

#define MAX_ARGS 100
#define MAX_CMD_LENGTH 1024
#define SANITIZED_CHARS ";&|><*?()$"

// Function declarations
void sanitize_input(char *input);
int split_args(char *input, char *args[]);
void handle_flags(int argc, char *argv[], int *arg_index, int *print_command, int *num_flag, int *n_value, char **placeholder, int *skip_execution);
void execute_command(char *args[], int print_command);
void process_num_flag(char *base_args[], int base_arg_count, char *input_args[], int additional_args, int n_value, int print_command);
void process_placeholder_flag(char *base_args[], int base_arg_count, char *input_args[], int additional_args, char *placeholder, int print_command);
void process_no_num_flag(char *base_args[], int base_arg_count, char *input_args[], int additional_args, int print_command);

// Function to remove special characters from input
void sanitize_input(char *input) {
    char sanitized[MAX_CMD_LENGTH] = "";
    int j = 0;
    
    for (int i = 0; input[i] != '\0'; i++) {
        if (strchr(SANITIZED_CHARS, input[i]) == NULL) {
            sanitized[j++] = input[i];
        }
    }
    sanitized[j] = '\0';
    strcpy(input, sanitized);
}

// Function to split a string into arguments
int split_args(char *input, char *args[]) {
    int count = 0;
    char *token = strtok(input, " ");
    while (token != NULL && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = strtok(NULL, " ");
    }
    args[count] = NULL;
    return count;
}

// Function to handle flags and options
void handle_flags(int argc, char *argv[], int *arg_index, int *print_command, int *num_flag, int *n_value, char **placeholder, int *skip_execution) {
    while (*arg_index < argc && argv[*arg_index][0] == '-') {
        switch (argv[*arg_index][1]) {
            case 't':
                *print_command = 1;
                (*arg_index)++;
                break;
            case 'n':
                *num_flag = 1;
                (*arg_index)++;
                if (*arg_index < argc) {
                    *n_value = atoi(argv[*arg_index]);
                    if (*n_value <= 0) {
                        fprintf(stderr, "Error: Invalid value for -n. Must be a positive integer.\n");
                        exit(1);
                    }
                    (*arg_index)++;
                } else {
                    fprintf(stderr, "Error: -n flag requires a number.\n");
                    exit(1);
                }
                break;
            case 'I':
                (*arg_index)++;
                if (*arg_index < argc) {
                    *placeholder = argv[*arg_index];
                    (*arg_index)++;
                } else {
                    fprintf(stderr, "Error: -I flag requires a placeholder.\n");
                    exit(1);
                }
                break;
            case 'r':
                *skip_execution = 1;
                (*arg_index)++;
                break;
            default:
                fprintf(stderr, "Error: Unknown flag %s\n", argv[*arg_index]);
                exit(1);
        }
    }
}

// Function to execute a command
void execute_command(char *args[], int print_command) {
    if (print_command) {
        printf("+ ");
        for (int i = 0; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else {
        wait(NULL);
    }
}

// Function to handle the -n flag
void process_num_flag(char *base_args[], int base_arg_count, char *input_args[], int additional_args, int n_value, int print_command) {
    int start_index = 0;
    while (start_index < additional_args) {
        char *args[MAX_ARGS];
        int copy_count = 0;

        // Copy base arguments
        for (int j = 0; j < base_arg_count; j++) {
            args[copy_count++] = strdup(base_args[j]);
        }

        // Add the current batch of input arguments
        for (int j = start_index; j < start_index + n_value && j < additional_args; j++) {
            args[copy_count++] = strdup(input_args[j]);
        }
        args[copy_count] = NULL;

        // Execute the command
        execute_command(args, print_command);

        // Free allocated memory
        for (int j = 0; j < copy_count; j++) {
            free(args[j]);
        }

        start_index += n_value;
    }
}

// Function to handle the placeholder logic (-I flag)
void process_placeholder_flag(char *base_args[], int base_arg_count, char *input_args[], int additional_args, char *placeholder, int print_command) {
    char modified_cmd[MAX_CMD_LENGTH];
    char all_args[MAX_CMD_LENGTH] = "";

    for (int i = 0; i < additional_args; i++) {
        strcat(all_args, input_args[i]);
        if (i < additional_args - 1) {
            strcat(all_args, " ");
        }
    }

    strcpy(modified_cmd, base_args[0]);
    for (int j = 1; j < base_arg_count; j++) {
        strcat(modified_cmd, " ");
        strcat(modified_cmd, base_args[j]);
    }

    char *pos;
    while ((pos = strstr(modified_cmd, placeholder)) != NULL) {
        size_t len = strlen(placeholder);
        char temp[MAX_CMD_LENGTH];
        strncpy(temp, modified_cmd, pos - modified_cmd);
        temp[pos - modified_cmd] = '\0';
        strcat(temp, all_args);
        strcat(temp, pos + len);
        strcpy(modified_cmd, temp);
    }

    char *args[MAX_ARGS];
    split_args(modified_cmd, args);

    // Execute the command if the placeholder was replaced
    if (strstr(modified_cmd, placeholder) == NULL) {
        execute_command(args, print_command);
    }
}

// Function to handle when -n flag is not used
void process_no_num_flag(char *base_args[], int base_arg_count, char *input_args[], int additional_args, int print_command) {
    for (int i = 0; i < additional_args; i++) {
        char *args[MAX_ARGS];
        int copy_count = 0;

        // Copy the base arguments
        for (int j = 0; j < base_arg_count; j++) {
            args[copy_count++] = strdup(base_args[j]);
        }

        // Add the current input argument
        args[copy_count++] = strdup(input_args[i]);
        args[copy_count] = NULL;

        // Execute the command
        execute_command(args, print_command);

        // Free allocated memory
        for (int j = 0; j < copy_count; j++) {
            free(args[j]);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: myxargs [-n num] [-I replace] [-t] [-r] command\n");
        exit(1);
    }

    int print_command = 0, num_flag = 0, n_value = 0, skip_execution = 0;
    char *placeholder = NULL;
    int arg_index = 1;

    // Handle flags
    handle_flags(argc, argv, &arg_index, &print_command, &num_flag, &n_value, &placeholder, &skip_execution);

    // Prepare the base command arguments
    char *base_args[MAX_ARGS];
    int base_arg_count = 0;
    for (int i = arg_index; i < argc; i++) {
        base_args[base_arg_count++] = argv[i];
    }
    base_args[base_arg_count] = NULL;

    // Sanitize base arguments
    for (int i = 0; i < base_arg_count; i++) {
        sanitize_input(base_args[i]);
    }

    char cmd[MAX_CMD_LENGTH];
    int input_empty = 1;

    // Handle input from stdin
    while (fgets(cmd, sizeof(cmd), stdin) != NULL) {
        cmd[strcspn(cmd, "\n")] = 0; // Remove newline

        if (strlen(cmd) > 0) {
            input_empty = 0;
        }

        sanitize_input(cmd);

        // Split the input into arguments
        char *input_args[MAX_ARGS];
        int additional_args = split_args(cmd, input_args);

        if (num_flag) {
            process_num_flag(base_args, base_arg_count, input_args, additional_args, n_value, print_command);
        } else if (placeholder) {
            process_placeholder_flag(base_args, base_arg_count, input_args, additional_args, placeholder, print_command);
        } else {
            process_no_num_flag(base_args, base_arg_count, input_args, additional_args, print_command);
        }
    }

    // If input is empty and -r flag is set, skip execution
    if (input_empty && skip_execution) {
        return 0;
    }

    // If no input and no -n flag, execute base command without arguments
    if (input_empty && !num_flag) {
        execute_command(base_args, print_command);
    }

    return 0;
}
