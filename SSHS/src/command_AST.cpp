#include "../include/command_AST.h"

struct CommandResult {
    std::string output;
    int exitStatus;
    bool processed {false};
};

struct TreeNode {
    std::string value;
    std::string output;
    TreeNode *left, *right;

    explicit TreeNode(std::string val) : value(std::move(val)), left(nullptr), right(nullptr) {}
};

void printPostOrder(TreeNode* node) {
    if (node == nullptr) {
        return;
    }

    // First, traverse the left subtree
    printPostOrder(node->left);

    // Then, traverse the right subtree
    printPostOrder(node->right);

    // Finally, visit the node itself
    std::cout << node->value << " ";
}


TreeNode* constructAST(const std::vector<std::string>& postfix) {
    std::stack<TreeNode*> stack;

    for (const auto& token : postfix) {
        if (isOperator(token)) {
            auto* node = new TreeNode(token);

            if (!stack.empty()) {
                node->right = stack.top();
                stack.pop();
            }

            if (!stack.empty()) {
                node->left = stack.top();
                stack.pop();
            }

            stack.push(node);
        } else {
            stack.push(new TreeNode(token));
        }
    }

    return stack.empty() ? nullptr : stack.top();
}

bool isExecutionSuccessful(const CommandResult& result) {
    return result.exitStatus == 0;
}

CommandResult traverseAndExecute(TreeNode* node, std::string& path) {
    CommandResult result;

    if (node == nullptr) {
        return { "", 0 };
    }

    if (!isOperator(node->value)) {
        return { node->value, 0 };
    }

    CommandResult leftResult = traverseAndExecute(node->left, path);
    CommandResult rightResult = traverseAndExecute(node->right, path);

    if (node->value == "|") {
        return execute_pipe_command(leftResult, rightResult, path);
    }
    else if (node->value == ">") {
        return redirect_output_to_file(leftResult, rightResult.output, path);
    }
    else if (node->value == "<") {
        return redirect_input_from_file(leftResult.output, rightResult.output, path);
    }
    else if (node->value == "2>") {
        return redirect_stderr_to_file(leftResult, rightResult, path);
    }
    else if (node->value == "&&") {
        CommandResult cmdResult;
        if (leftResult.processed && leftResult.exitStatus == 0) {
            if (!rightResult.processed) {
                CommandResult right = execute_command(rightResult.output, path);
                cmdResult.output = leftResult.output + right.output;
                cmdResult.exitStatus = right.exitStatus;
                cmdResult.processed = true;
                return cmdResult;
            }
            else {
                cmdResult.output = leftResult.output + rightResult.output;
                cmdResult.exitStatus = rightResult.exitStatus;
                cmdResult.processed = true;
                return cmdResult;
            }
        }
        else if (leftResult.processed && leftResult.exitStatus != 0) {
            return leftResult;
        }
        else if (!leftResult.processed) {
            CommandResult left = execute_command(leftResult.output, path);
            if (left.exitStatus == 0) {
                CommandResult right = execute_command(rightResult.output, path);
                cmdResult.output = left.output + right.output;
                cmdResult.exitStatus = right.exitStatus;
                cmdResult.processed = true;
                return cmdResult;
            }
            else if (left.exitStatus != 0) {
                left.processed = true;
                return left;
            }
        }
    }
    else if (node->value == "||") {
        CommandResult cmdResult;
        if (leftResult.processed && leftResult.exitStatus != 0) {
            if (!rightResult.processed) {
                CommandResult right = execute_command(rightResult.output, path);
                cmdResult.output = leftResult.output + right.output;
                cmdResult.exitStatus = right.exitStatus;
                return cmdResult;
            }
            else {
                cmdResult.output = leftResult.output + rightResult.output;
                cmdResult.exitStatus = rightResult.exitStatus;
                return cmdResult;
            }
        }
        else if (leftResult.processed && leftResult.exitStatus == 0) {
            return leftResult;
        }
        else if (!leftResult.processed) {
            CommandResult left = execute_command(leftResult.output, path);
            if (left.exitStatus != 0) {
                if(!rightResult.processed) {
                    CommandResult right = execute_command(rightResult.output, path);
                    cmdResult.output = left.output + right.output;
                    cmdResult.exitStatus = right.exitStatus;
                    return cmdResult;
                }
                else {
                    cmdResult.output = left.output + rightResult.output;
                    cmdResult.exitStatus = rightResult.exitStatus;
                    return cmdResult;
                }
            }
            else if (left.exitStatus == 0) {
                return left;
            }
        }
    }
    else if (node->value == ";") {
        CommandResult cmdResult;
        if (!leftResult.processed) {
            CommandResult left = execute_command(leftResult.output, path);
            if (!rightResult.processed) {
                CommandResult right = execute_command(rightResult.output, path);
                cmdResult.output = left.output + right.output;
                return  cmdResult;
            }
            else if (rightResult.processed) {
                cmdResult.output = left.output + rightResult.output;
                return cmdResult;
            }
        }
        else if (leftResult.processed) {
            if (!rightResult.processed) {
                CommandResult right = execute_command(rightResult.output, path);
                cmdResult.output = leftResult.output + right.output;
                return cmdResult;
            }
            else if (rightResult.processed) {
                cmdResult.output = leftResult.output + rightResult.output;
                return cmdResult;
            }
        }
    }

    result.output = "This should not be reached.";
    result.exitStatus = 1;;
    return result;
}