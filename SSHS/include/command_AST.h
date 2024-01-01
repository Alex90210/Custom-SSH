#ifndef COMMAND_AST_H
#define COMMAND_AST_H

#include <string>
#include <vector>
#include <stack>
#include "infix_to_postfix.h"
#include "exec_command.h"
#include "multiple_commands.h"

struct CommandResult;
struct TreeNode;
void printPostOrder(TreeNode* node);
TreeNode* constructAST(const std::vector<std::string>& postfix);
CommandResult traverseAndExecute(TreeNode* node, std::string& path);

#endif