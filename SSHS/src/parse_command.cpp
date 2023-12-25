#include "../include/parse_command.h"

std::vector<std::string> splitCommandString(const std::string& commandStr) {
    std::vector<std::string> commands;
    std::istringstream stream(commandStr);
    std::string command;

    while (std::getline(stream, command, '|')) {
        // Trim leading and trailing spaces from each command
        size_t start = command.find_first_not_of(" \t");
        size_t end = command.find_last_not_of(" \t");

        if (start != std::string::npos) {
            commands.push_back(command.substr(start, end - start + 1));
        }
    }

    return commands;
}

void parseAndExecute(const std::string& inputCommand) {
    // Step 1: Parse the command string
    // This involves breaking the command into tokens and handling redirections, pipes, and logical operators

    // Step 2: Handle Redirections
    // Setup input/output redirection if specified

    // Step 3: Handle Pipes
    // If there are pipes, create them and setup the necessary subprocesses

    // Step 4: Handle Logical Operators
    // Implement logic for && and ||

    // Step 5: Sequential Execution for ;
    // Run commands separated by ; sequentially

    // Note: This is a high-level approach. Each step involves detailed implementation.
}

std::string executePipeline(const std::string& commandStr) {
    std::vector<std::string> commands = splitCommandString(commandStr);
    size_t numCommands = commands.size();
    if (numCommands == 0) {
        return "No command to execute.";
    }

    std::vector<int> pfd(2 * (numCommands - 1));
    for (size_t i = 0; i < numCommands - 1; ++i) {
        if (pipe(pfd.data() + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (size_t i = 0; i < numCommands; ++i) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            if (i > 0) {
                dup2(pfd[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < numCommands - 1) {
                dup2(pfd[i * 2 + 1], STDOUT_FILENO);
            }

            for (auto& fd : pfd) {
                close(fd);
            }

            execlp("/bin/sh", "sh", "-c", commands[i].c_str(), NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        }
    }

    for (auto& fd : pfd) {
        close(fd);
    }

    int status;
    for (size_t i = 0; i < numCommands; ++i) {
        wait(&status);
    }

    // Read the output of the last command
    std::string finalOutput;
    char buffer[4096];
    ssize_t bytesRead;

    while ((bytesRead = read(pfd[(numCommands - 2) * 2], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        finalOutput += buffer;
    }

    return finalOutput;
}

void executeCommandWithRedirection(const std::string& command, const std::string& inputFile, const std::string& outputFile) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // Child process
        if (!inputFile.empty()) {
            int in_fd = open(inputFile.c_str(), O_RDONLY);
            if (in_fd == -1) {
                perror("open input file");
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (!outputFile.empty()) {
            int out_fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd == -1) {
                perror("open output file");
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        execlp("/bin/sh", "sh", "-c", command.c_str(), NULL);
        perror("execlp"); // execlp only returns on error
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        waitpid(pid, NULL, 0);
    }
}