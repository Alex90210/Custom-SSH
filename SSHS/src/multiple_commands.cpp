#include "../include/multiple_commands.h"

#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>

struct CommandResult {
    std::string output;
    int exitStatus {};
    bool processed {false};
};

bool is_cd_command (const std::string& command, std::string& path) {
    if (command == "cd") {
        path = "/home/alex";
        return true;
    }
    else if (command.substr(0, 3) == "cd ") {

        std::string parsed_path = command.substr(3, command.length());
        std::string temp_path1 = path + "/" + command.substr(3, command.length());
        std::string temp_path2 = path + command.substr(3, command.length());

        if (is_path_valid(temp_path1)) {
            path = temp_path1;
            return true;
        }
        else if (is_path_valid(temp_path2)) {
            path = temp_path2;
            return true;
        }
        else {
            std::cout << "Invalid path.\n";
            return false;
        }
    }

    return false;
}

CommandResult execute_command(const std::string& command, std::string& path) {
    CommandResult result;

    if (is_cd_command(command, path)) {
        result.processed = true;
        result.exitStatus = 0;
        result.output = path;
        return result;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return {"Error creating pipe", -1};
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return {"Fork failed", -1};
    }

    if (pid == 0) {
        // Child process
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO); // Optionally, redirect stderr to the pipe as well
        close(pipefd[1]);

        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }

        execlp("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    // exec_from_path_or_cd(command, path);
    // Parent process
    close(pipefd[1]); // Close unused write end
    std::string output;
    char buffer[4096];
    ssize_t count;

    while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[count] = '\0';
        output += buffer;
    }
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    // CommandResult result;
    result.processed = true;
    result.output = output;
    result.exitStatus = WEXITSTATUS(status);
    return result;
}

// this doesn't capture the error command, but it should
// ASAP: fix it
CommandResult execute_pipe_command(const CommandResult& leftCmd, const CommandResult& rightCmd, std::string& path) {

    CommandResult cmdResult;

    int pipefd[2];  // Pipe between leftCmd and rightCmd
    int outputfd[2]; // Pipe to capture the output of rightCmd
    pid_t leftPid, rightPid;
    const int bufferSize = 16384;
    char buffer[bufferSize];

    if (pipe(pipefd) == -1 || pipe(outputfd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    leftPid = fork();
    if (leftPid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (leftPid == 0) {  // Child process for leftCmd
        if (leftCmd.processed) {
            ssize_t written = write(pipefd[1], leftCmd.output.c_str(), leftCmd.output.size());
            if (written == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS); // Terminate the process since we've already sent the data
        }
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        close(outputfd[0]);
        close(outputfd[1]);
        execlp("/bin/sh", "sh", "-c", leftCmd.output.c_str(), (char *)NULL);
        perror("execlp leftCmd");
        exit(EXIT_FAILURE);
    }

    rightPid = fork();
    if (rightPid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (rightPid == 0) {  // Child process for rightCmd
        if (rightCmd.processed) {
            ssize_t written = write(outputfd[1], rightCmd.output.c_str(), rightCmd.output.size());
            if (written == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS); // Terminate the process since we've already sent the data
        }
        dup2(pipefd[0], STDIN_FILENO);
        dup2(outputfd[1], STDOUT_FILENO);
        dup2(outputfd[1], STDERR_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        close(outputfd[0]);
        close(outputfd[1]);

        execlp("/bin/sh", "sh", "-c", rightCmd.output.c_str(), (char *)NULL);
        perror("execlp rightCmd");
        exit(EXIT_FAILURE);
    }

    // Close the unused ends of the pipes in the parent process
    close(pipefd[0]);
    close(pipefd[1]);
    close(outputfd[1]);

    // Read the output from the second pipe
    std::string result;
    ssize_t count;
    while ((count = read(outputfd[0], buffer, bufferSize - 1)) > 0) {
        buffer[count] = '\0';
        result += buffer;
    }
    close(outputfd[0]);

    // Wait for the child processes to finish and capture the exit status
    int status;
    waitpid(rightPid, &status, 0); // We are primarily interested in the right command's status
    waitpid(leftPid, NULL, 0); // We still need to wait for the left command, but we don't need its status

    // Construct and return the CommandResult
    cmdResult.processed = true;
    cmdResult.output = result;
    cmdResult.exitStatus = WEXITSTATUS(status); // Extract the exit status

    return cmdResult;
}

CommandResult redirect_output_to_file(const CommandResult& input, const std::string& filePath, std::string& path) {
    CommandResult result;

    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        result.output = "Error opening file";
        result.exitStatus = errno; // Capture the error number
        return result;
    }

    ssize_t bytes_written;
    if (input.processed) {
        bytes_written = write(fd, input.output.c_str(), input.output.size());
    } else {
        result = execute_command(input.output, path);
        bytes_written = write(fd, result.output.c_str(), result.output.size());
    }

    if (bytes_written == -1) {
        perror("write");
        result.output = "Error writing to file";
        result.exitStatus = errno; // Capture the error number
    }

    // Close the file descriptor
    if (close(fd) == -1) {
        perror("close");
        result.output = "Error closing file";
        result.exitStatus = errno; // Capture the error number
    }

    result.processed = (result.exitStatus == 0);
    result.output = ""; // No output to return for a redirection operation
    return result;
}


CommandResult redirect_input_from_file(const std::string& command, const std::string& filePath, std::string& path) {
    CommandResult result;

    int filefd = open(filePath.c_str(), O_RDONLY);
    if (filefd == -1) {
        perror("open");
        result.output = "Error opening file";
        result.exitStatus = errno;
        return result;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        close(filefd);
        result.output = "Error creating pipe";
        result.exitStatus = errno;
        return result;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(filefd);
        close(pipefd[0]);
        close(pipefd[1]);
        result.output = "Fork failed";
        result.exitStatus = errno;
        return result;
    }

    if (pid == 0) {
        // Child process
        dup2(filefd, STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(filefd);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(filefd);
    close(pipefd[1]);

    // Read the output from the pipe
    std::string output;
    char buffer[4096];
    ssize_t count;
    while ((count = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[count] = '\0';
        output += buffer;
    }

    close(pipefd[0]);

    // Wait for the child process to finish and capture its exit status
    int status;
    waitpid(pid, &status, 0);

    result.processed = true;
    result.output = output;
    result.exitStatus = WEXITSTATUS(status);
    return result;
}

// this should be executed with a proper command
CommandResult redirect_stderr_to_file(const CommandResult& command, const CommandResult& filePath, std::string& path) {
    CommandResult result;

    int fd = open(filePath.output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        result.output = "Error opening file";
        result.exitStatus = errno;
        return result;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(fd);
        result.output = "Fork failed";
        result.exitStatus = errno;
        return result;
    }

    if (pid == 0) {
        // Child process
        if (command.processed) {
            ssize_t written = write(fd, command.output.c_str(), command.output.size());
            if (written == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            close(fd); // Close the file descriptor
            exit(EXIT_SUCCESS); // Terminate the process successfully
        }
        dup2(fd, STDERR_FILENO); // Redirect stderr to file
        close(fd);

        execlp("/bin/sh", "sh", "-c", command.output.c_str(), (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    // Parent process
    close(fd);

    // Wait for the child process to finish and capture its exit status
    int status;
    waitpid(pid, &status, 0);

    result.processed = true;
    result.output = ""; // No output to capture for stderr redirection
    result.exitStatus = WEXITSTATUS(status);
    return result;
}