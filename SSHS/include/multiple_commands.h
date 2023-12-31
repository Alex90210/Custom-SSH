#ifndef MULTIPLE_COMMANDS_H
#define MULTIPLE_COMMANDS_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "exec_command.h"
#include "command_AST.h"

struct CommandResult;
bool isBashExecutable(const std::string& cmd);
CommandResult execute_command(const std::string& command);
CommandResult execute_pipe_command(const CommandResult& leftCmd, const CommandResult& rightCmd, const std::string& path);
CommandResult redirectOutputToFile(const std::string& command, const std::string& filePath);
CommandResult redirectInputFromFile(const std::string& command, const std::string& filePath);
CommandResult redirectStderrToFile(const CommandResult& command, const CommandResult& filePath);

#endif