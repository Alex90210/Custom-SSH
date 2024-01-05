#ifndef MULTIPLE_COMMANDS_H
#define MULTIPLE_COMMANDS_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "exec_command.h"
#include "command_AST.h"

struct CommandResult;
std::string exec_from_path_or_cd(const std::string& command, std::string& path);
bool is_cd_command (const std::string& command, std::string& path);
CommandResult execute_command(const std::string& command, std::string& path);
CommandResult execute_pipe_command(const CommandResult& left_cmd, const CommandResult& right_cmd, std::string& path);
CommandResult redirect_output_to_file(const CommandResult& command, const std::string& file_path, std::string& path);
CommandResult redirect_input_from_file(const std::string& command, const std::string&, std::string& path);
CommandResult redirect_stderr_to_file(const CommandResult& command, const std::string& file_path, std::string& path);

#endif