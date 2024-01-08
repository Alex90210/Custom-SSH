#include "../include/multiple_commands.h"

#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>

struct CommandResult {
    std::string output;
    int exit_status {};
    bool processed {false};
};

bool is_cd_command(const std::string& command, std::string& path) {
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
        result.processed = false; // The cd does not output anything, this should skip this step
        result.exit_status = 0;
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
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }
        execlp("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    close(pipefd[1]);

    std::string output;
    std::vector<char> buffer(16384);
    ssize_t count;
    while ((count = read(pipefd[0], buffer.data(), buffer.size() - 1)) > 0) {
        buffer[count] = '\0';
        output.append(buffer.data());
    }

    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    result.processed = true;
    result.output = output;
    result.exit_status = WEXITSTATUS(status);
    return result;
}

CommandResult execute_pipe_command(const CommandResult& left_cmd, const CommandResult& right_cmd, std::string& path) {
    CommandResult cmdResult;

    int pipefd[2];
    int outputfd[2];
    pid_t left_pid, right_pid;

    if (pipe(pipefd) == -1 || pipe(outputfd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    left_pid = fork();
    if (left_pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (left_pid == 0) {
        if (left_cmd.processed) {
            ssize_t written = write(pipefd[1], left_cmd.output.c_str(), left_cmd.output.size());
            if (written == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        close(outputfd[0]);
        close(outputfd[1]);
        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }
        execlp("/bin/sh", "sh", "-c", left_cmd.output.c_str(), NULL);
        perror("execlp leftCmd");
        exit(EXIT_FAILURE);
    }

    right_pid = fork();
    if (right_pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (right_pid == 0) {
        if (right_cmd.processed) {
            ssize_t written = write(outputfd[1], right_cmd.output.c_str(), right_cmd.output.size());
            if (written == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        dup2(pipefd[0], STDIN_FILENO);
        dup2(outputfd[1], STDOUT_FILENO);
        dup2(outputfd[1], STDERR_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        close(outputfd[0]);
        close(outputfd[1]);
        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }
        execlp("/bin/sh", "sh", "-c", right_cmd.output.c_str(), (char *)NULL);
        perror("execlp rightCmd");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    close(outputfd[1]);

    std::string result;
    std::vector<char> buffer(16384);
    ssize_t count;
    while ((count = read(outputfd[0], buffer.data(), buffer.size() - 1)) > 0) {
        buffer[count] = '\0';
        result.append(buffer.data());
    }
    close(outputfd[0]);

    int status;
    waitpid(right_pid, &status, 0);
    waitpid(left_pid, NULL, 0);

    cmdResult.processed = true;
    cmdResult.output = result;
    cmdResult.exit_status = WEXITSTATUS(status);

    return cmdResult;
}

CommandResult redirect_output_to_file(const CommandResult& input, const std::string& file_path, std::string& path) {
    CommandResult result;

    std::string full_path;
    if (file_path.substr(0, 1) == "/") {
        full_path = file_path;
    }
    else {
        full_path = path + "/" + file_path;
    }

    int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        result.output = "Error opening file";
        result.exit_status = errno;
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
        result.exit_status = errno;
    }

    if (close(fd) == -1) {
        perror("close");
        result.output = "Error closing file";
        result.exit_status = errno;
    }

    result.processed = (result.exit_status == 0);
    result.output = "";
    return result;
}

CommandResult redirect_input_from_file(const std::string& command, const std::string& file_path, std::string& path) {
    CommandResult result;

    std::string full_path = (file_path.front() == '/') ? file_path : path + "/" + file_path;

    int filefd = open(full_path.c_str(), O_RDONLY);
    if (filefd == -1) {
        perror("open");
        result.output = "Error opening file";
        result.exit_status = errno;
        return result;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        close(filefd);
        result.output = "Error creating pipe";
        result.exit_status = errno;
        return result;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(filefd);
        close(pipefd[0]);
        close(pipefd[1]);
        result.output = "Fork failed";
        result.exit_status = errno;
        return result;
    }

    if (pid == 0) {
        dup2(filefd, STDIN_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(filefd);
        close(pipefd[0]);
        close(pipefd[1]);
        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }
        execlp("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    close(filefd);
    close(pipefd[1]);

    std::string output;
    std::vector<char> buffer(16384);
    ssize_t count;
    while ((count = read(pipefd[0], buffer.data(), buffer.size() - 1)) > 0) {
        buffer[count] = '\0';
        output.append(buffer.data());
    }

    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    result.processed = true;
    result.output = output;
    result.exit_status = WEXITSTATUS(status);
    return result;
}

CommandResult redirect_stderr_to_file(const CommandResult& command, const std::string& file_path, std::string& path) {
    CommandResult result;

    std::string full_path;
    if (file_path.substr(0, 1) == "/") {
        full_path = file_path;
    }
    else {
        full_path = path + "/" + file_path;
    }

    int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open");
        result.output = "Error opening file";
        result.exit_status = errno;
        return result;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(fd);
        result.output = "Fork failed";
        result.exit_status = errno;
        return result;
    }

    if (pid == 0) {
        if (command.processed) {
            ssize_t written = write(fd, command.output.c_str(), command.output.size());
            if (written == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            close(fd);
            exit(EXIT_SUCCESS);
        }
        dup2(fd, STDERR_FILENO);
        close(fd);

        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }

        execlp("/bin/sh", "sh", "-c", command.output.c_str(), (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    close(fd);

    int status;
    waitpid(pid, &status, 0);

    result.processed = true;
    result.output = "";
    result.exit_status = WEXITSTATUS(status);
    return result;
}