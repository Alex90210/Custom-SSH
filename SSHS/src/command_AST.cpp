#include "../include/command_AST.h"

struct CommandResult {
    std::string output;
    int exitStatus;
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
        result.output = "";
        result.exitStatus = 0;
        return result;
    }

    if (!isOperator(node->value)) {
        result.output = node->value;
        result.exitStatus = 0;
        return result;
    }

    CommandResult leftResult = traverseAndExecute(node->left, path);
    CommandResult rightResult = traverseAndExecute(node->right, path);

    if (node->value == "|") {
        return execute_pipe_command(leftResult.output, rightResult.output, path);
    }
    else if (node->value == ">") {
        return redirectOutputToFile(leftResult.output, rightResult.output);
    }
    else if (node->value == "<") {
        return redirectInputFromFile(leftResult.output, rightResult.output);
    }
    else if (node->value == "2>") {
        return redirectStderrToFile(leftResult.output, rightResult.output);
    }
    if (node->value == "&&") {
        CommandResult left = execute_command(node->left->value);
        if (left.exitStatus == 0) {
            CommandResult right = execute_command(node->right->value);
            std::string full_output = left.output + '\n' + right.output;
            CommandResult full_result;
            full_result.output = full_output;
            full_result.exitStatus = right.exitStatus;
            return full_result;
        }
    }
    if (node->value == "||") {
        CommandResult left = execute_command(node->left->value);
        if (left.exitStatus == 1) {
            CommandResult right = execute_command(node->right->value);
            return right;
        }
        return left;
    }
    if (node->value == ";") {
        CommandResult left = execute_command(node->left->value);
        CommandResult right = execute_command(node->right->value);
        std::string full_output = left.output + '\n' + right.output;
        CommandResult full_result;
        full_result.output = full_output;
        full_result.exitStatus = right.exitStatus;
        return full_result;
    }


    result.output = "";
    result.exitStatus = 0;
    return result;
}