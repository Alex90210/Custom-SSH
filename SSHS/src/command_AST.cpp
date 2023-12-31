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

CommandResult traverseAndExecute(TreeNode* node, const std::string& path) {
    CommandResult result;

    if (node == nullptr) {
        return { "", 0 };
    }

    // Directly return the command for leaf nodes
    if (!isOperator(node->value)) {
        return { node->value, 0 };
    }

    // Recursively process left and right subtrees
    CommandResult leftResult = traverseAndExecute(node->left, path);
    CommandResult rightResult = traverseAndExecute(node->right, path);

    if (node->value == "|") {
        return execute_pipe_command(leftResult, rightResult, path);
    }
    else if (node->value == ">") {
        return redirectOutputToFile(leftResult.output, rightResult.output);
    }
    else if (node->value == "<") {
        return redirectInputFromFile(leftResult.output, rightResult.output);
    }
    else if (node->value == "2>") {
        return redirectStderrToFile(leftResult, rightResult);
    }
    else if (node->value == "&&") {
        CommandResult cmdResult;
        cmdResult = execute_command(leftResult.output);
        if (cmdResult.exitStatus == 0) {
            cmdResult = execute_command(rightResult.output);
            return cmdResult;
        }
        return cmdResult;
    }
    else if (node->value == "||") {
        CommandResult cmdResult;
        cmdResult = execute_command(leftResult.output);
        if (cmdResult.exitStatus != 0) {
            cmdResult = execute_command(rightResult.output);
            return cmdResult;
        }
        return cmdResult;
    }
    else if (node->value == ";") {
        CommandResult cmdResult;
        // CommandResult first_expr = execute_command(leftResult.output);
        // CommandResult second_expr = execute_command(rightResult.output);
        // std::string full_output = first_expr.output + second_expr.output;
        if (isBashExecutable(leftResult.output)) {
            cmdResult = execute_command(leftResult.output);
        } else {
            cmdResult.output = leftResult.output;
        }
        if (isBashExecutable(rightResult.output)) {
            cmdResult = execute_command(rightResult.output);
        } else {
            cmdResult.output += " " + rightResult.output;
        }

        /*std::string full_output = leftResult.output + rightResult.output;
        cmdResult.output = full_output;
        cmdResult.exitStatus = rightResult.exitStatus;*/
        return cmdResult;
    }

    // Default case if an unknown operator is encountered
    return { "Unknown operator: " + node->value, 1 };
}