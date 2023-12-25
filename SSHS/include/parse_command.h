#ifndef PARSE_COMMAND_H
#define PARSE_COMMAND_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "exec_command.h"

std::vector<std::string> splitCommandString(const std::string& commandStr);
std::string executePipeline(const std::string& commandStr);
void executeCommandWithRedirection(const std::string& command, const std::string& inputFile, const std::string& outputFile);

#endif