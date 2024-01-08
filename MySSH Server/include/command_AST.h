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
void print_post_order(TreeNode* node);
TreeNode* construct_AST(const std::vector<std::string>& postfix);
CommandResult traverse_and_execute(TreeNode* node, std::string& path);

#endif