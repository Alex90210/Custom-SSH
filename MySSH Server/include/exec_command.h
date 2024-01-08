#ifndef EXEC_COMMAND_H
#define EXEC_COMMAND_H

#include <nlohmann/json.hpp>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <climits>
#include <dirent.h>
#include "multiple_commands.h"
#include "infix_to_postfix.h"

bool is_path_valid(const std::string& path);
std::string interpret_command(const std::string& command, std::string& path);
std::string execute_cmd_without_operators(const std::vector<std::string>& vec_command,
                                          std::string& path);

#endif