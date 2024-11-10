# Custom Command Line Argument Processor (`myxargs`)

## Description
This program, inspired by the Unix `xargs` command, reads input arguments from standard input (stdin) and uses them to execute a specified command. It allows users to handle complex argument configurations and supports multiple flags to customize execution behavior. This program enables batch processing of arguments, placeholder substitutions, and command preview functionality, offering flexible automation for command-line tasks.

## Key Features
- **Batch Argument Execution**: 
  - With the `-n` flag, users can specify the number of arguments to pass per execution, allowing for batch processing.
- **Placeholder Substitution**: 
  - Using the `-I` flag, this program replaces placeholders in the command with input arguments, enabling dynamic input insertion.
- **Command Preview**: 
  - The `-t` flag prints the command before execution, aiding in debugging and tracking.
- **Execution Control**: 
  - The `-r` flag prevents command execution if no arguments are provided, adding an extra layer of control.
- **Input Sanitization**: 
  - Removes potentially unsafe characters (like `&`, `|`, `;`, etc.) from input arguments to improve security.

## Usage
- **Basic Execution**: Run commands with arguments passed from standard input.
- **Flags for Customization**: Options like `-n`, `-I`, `-t`, and `-r` enable tailored command executions.
- **Real-World Applications**: Useful for automating repetitive command executions, such as batch file processing, network requests, and system maintenance scripts.

### Flags
- `-n <num>`: Specifies the number of arguments to pass per execution.
- `-I <replace>`: Replaces instances of `<replace>` in the command with input arguments.
- `-t`: Prints the command to be executed.
- `-r`: Skips execution if no arguments are provided.

## Examples
```bash
# List files in batches of two
echo "file1 file2 file3" | ./myxargs -n 2 ls
# Output:
# ls file1 file2
# ls file3

# Copy directories to /backup, replacing {} with each argument
echo "dir1 dir2" | ./myxargs -I {} cp {} /backup
# Output:
# cp dir1 /backup
# cp dir2 /backup
