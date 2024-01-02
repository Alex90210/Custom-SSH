#include "../include/exec_command.h"

struct CommandResult {
    std::string output;
    int exit_status;
};

bool is_path_valid(const std::string& path) {
    std::filesystem::path fs_path(path);
    return std::filesystem::exists(fs_path) && std::filesystem::is_directory(fs_path);
}

std::string interpret_command(const std::string& command, std::string& path) {
    std::string command_output;

    if (command == "cd" && !contains_bash_operator(command)) {
        path = "/home/alex";
    }
    if (command.substr(0, 3) == "cd " && !contains_bash_operator(command)) {
        std::string parsed_path = command.substr(3, command.length());
        std::string temp_path1 = path + "/" + command.substr(3, command.length());
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
        if (!contains_bash_operator(command)) {
            command_output = main_execute_command(command, path);
        }
        else {
            std::vector<std::string> tokens = tokenize(command);
            std::vector<std::string> postfix = convert_to_postfix(tokens);
            TreeNode* root = construct_AST(postfix);
            // printPostOrder(root);
            CommandResult output = traverse_and_execute(root, path);
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
    } else if (pid > 0) {
        close(pipe_fd[1]);

        int status;
        waitpid(pid, &status, 0);

        char buffer[16384];
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

        execlp("/bin/sh", "sh", "-c", command.c_str(), (char *)NULL);
        std::cerr << "Failed to execute command." << std::endl;
        exit(EXIT_FAILURE);
    }
}