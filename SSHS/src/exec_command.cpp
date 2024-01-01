#include "../include/exec_command.h"

struct CommandResult {
    std::string output;
    int exitStatus;
};

bool is_path_valid(const std::string& path) {
    std::filesystem::path fs_path(path);
    return std::filesystem::exists(fs_path) && std::filesystem::is_directory(fs_path);
}

std::string interpret_command(const std::string& command, std::string& path) {
    std::string command_output;

    if (command == "cd" && !containsBashOperator(command)) {
        path = "/home/alex";
        // command_output = path;
    }
    if (command.substr(0, 3) == "cd " && !containsBashOperator(command)) {
        std::string parsed_path = command.substr(3, command.length());
        std::string temp_path1 = path + "/" + command.substr(3, command.length());
        // std::string temp_path2 = path + command.substr(3, command.length());
        std::string temp_path2 = command.substr(3, command.length());

        if (is_path_valid(temp_path1)) {
            path = temp_path1;
        }
        else if (is_path_valid(temp_path2)) {
            path = temp_path2;
        }
        else {
            command_output = "Invalid path.\n";
        }
    }
    else if (command == "pwd") {
        command_output = path + "\n";
    }
    else {
        if (!containsBashOperator(command)) {
            command_output = main_execute_command(command, path);
        }
        else {
            std::vector<std::string> tokens = tokenize(command);
            std::vector<std::string> postfix = convertToPostfix(tokens);
            TreeNode* root = constructAST(postfix);
            // printPostOrder(root);
            CommandResult output = traverseAndExecute(root, path);
            command_output = output.output;
        }
    }

    return command_output;
}

std::string main_execute_command(const std::string& command, std::string& path) {
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
        // The second command freezes the program here
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