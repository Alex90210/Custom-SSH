#include "../include/exec_command.h"

bool is_path_valid(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

std::string interpret_command(const std::string& command, std::string& path) {

    // You should verify in the client if the command is cd
    // If it is, you should expect the server to send you the new path
    // and then update it accordingly
    std::string command_output;
    if (command == "cd") {
        path = "/home/alex";
        command_output = path;
    }
    if (command.substr(0, 3) == "cd ") {
        if (is_path_valid(command.substr(3, command.length()))) {
            path = command.substr(3, command.length());
            command_output = path;
        }
    }
    else {
        command_output = execute_command(command, path);
    }

    return command_output;
}

std::string execute_command(const std::string& command, std::string& path) {
    std::array<int, 2> pipe_fd{};
    pipe(pipe_fd.data());

    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "Failed to fork." << std::endl;
        exit(EXIT_FAILURE);
    } else if (pid > 0)
    {
        close(pipe_fd[1]);

        int status;
        waitpid(pid, &status, 0);

        char buffer[16384]; // 16KB
        std::string result;

        ssize_t count;
        while ((count = read(pipe_fd[0], buffer, sizeof(buffer)-1)) > 0) {
            buffer[count] = '\0';
            result += buffer;
        }

        close(pipe_fd[0]);
        return result;

    } else {

        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        if (chdir(path.c_str()) != 0) {
            std::cerr << "Failed to change directory." << std::endl;
            exit(EXIT_FAILURE);
        }

        // You should create a tree of commands and execute them in order
        execlp("bash", "bash", "-c", command.c_str(), NULL);
        std::cerr << "Failed to execute command." << std::endl;
        exit(EXIT_FAILURE);
    }
}

std::string get_current_directory() {
    size_t size = 1024;
    char *buffer = new (std::nothrow) char[size];
    if (buffer == nullptr) {
        std::cerr << "Unable to allocate buffer" << std::endl;
        return "";
    }

    while (getcwd(buffer, size) == nullptr) {
        size *= 2;
        char *newBuffer = new (std::nothrow) char[size];
        if (newBuffer == nullptr) {
            std::cerr << "Unable to reallocate buffer" << std::endl;
            delete[] buffer;
            return "";
        }
        delete[] buffer;
        buffer = newBuffer;
    }

    std::string currentDirectory(buffer);
    delete[] buffer;
    return currentDirectory;
}