#ifndef COMMAND_AST_H
#define COMMAND_AST_H

#include <string>
#include <vector>
#include <stack>
#include "infix_to_postfix.h"

struct TreeNode;
TreeNode* constructAST(const std::vector<std::string>& postfix);

#endif