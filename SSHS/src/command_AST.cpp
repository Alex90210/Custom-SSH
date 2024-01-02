#include "../include/command_AST.h"

struct CommandResult {
    std::string output;
    int exit_status {};
    bool processed {false};
};

struct TreeNode {
    std::string value;
    std::string output;
    TreeNode *left, *right;

    TreeNode(std::string val) : value(std::move(val)), left(nullptr), right(nullptr) {}
};

void print_post_order(TreeNode* node) {
    if (node == nullptr) {
        return;
    }

    print_post_order(node->left);
    print_post_order(node->right);
    std::cout << node->value << " ";
}


TreeNode* construct_AST(const std::vector<std::string>& postfix) {
    std::stack<TreeNode*> stack;

    for (const auto& token : postfix) {
        if (is_operator(token)) {
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

CommandResult traverse_and_execute(TreeNode* node, std::string& path) {
    CommandResult result;

    if (node == nullptr) {
        return { "", 0 };
    }

    if (!is_operator(node->value)) {
        return { node->value, 0 };
    }

    CommandResult left_result = traverse_and_execute(node->left, path);
    CommandResult right_result = traverse_and_execute(node->right, path);

    if (node->value == "|") {
        return execute_pipe_command(left_result, right_result, path);
    }
    else if (node->value == ">") {
        return redirect_output_to_file(left_result, right_result.output, path);
    }
    else if (node->value == "<") {
        return redirect_input_from_file(left_result.output, right_result.output, path);
    }
    else if (node->value == "2>") {
        return redirect_stderr_to_file(left_result, right_result, path);
    }
    else if (node->value == "&&") {
        CommandResult cmdResult;
        if (left_result.processed && left_result.exit_status == 0) {
            if (!right_result.processed) {
                CommandResult right = execute_command(right_result.output, path);
                cmdResult.output = left_result.output + right.output;
                cmdResult.exit_status = right.exit_status;
                cmdResult.processed = true;
                return cmdResult;
            }
            else {
                cmdResult.output = left_result.output + right_result.output;
                cmdResult.exit_status = right_result.exit_status;
                cmdResult.processed = true;
                return cmdResult;
            }
        }
        else if (left_result.processed && left_result.exit_status != 0) {
            return left_result;
        }
        else if (!left_result.processed) {
            CommandResult left = execute_command(left_result.output, path);
            if (left.exit_status == 0) {
                CommandResult right = execute_command(right_result.output, path);
                cmdResult.output = left.output + right.output;
                cmdResult.exit_status = right.exit_status;
                cmdResult.processed = true;
                return cmdResult;
            }
            else if (left.exit_status != 0) {
                left.processed = true;
                return left;
            }
        }
    }
    else if (node->value == "||") {
        CommandResult cmdResult;
        if (left_result.processed && left_result.exit_status != 0) {
            if (!right_result.processed) {
                CommandResult right = execute_command(right_result.output, path);
                cmdResult.output = left_result.output + right.output;
                cmdResult.exit_status = right.exit_status;
                return cmdResult;
            }
            else {
                cmdResult.output = left_result.output + right_result.output;
                cmdResult.exit_status = right_result.exit_status;
                return cmdResult;
            }
        }
        else if (left_result.processed && left_result.exit_status == 0) {
            return left_result;
        }
        else if (!left_result.processed) {
            CommandResult left = execute_command(left_result.output, path);
            if (left.exit_status != 0) {
                if(!right_result.processed) {
                    CommandResult right = execute_command(right_result.output, path);
                    cmdResult.output = left.output + right.output;
                    cmdResult.exit_status = right.exit_status;
                    return cmdResult;
                }
                else {
                    cmdResult.output = left.output + right_result.output;
                    cmdResult.exit_status = right_result.exit_status;
                    return cmdResult;
                }
            }
            else if (left.exit_status == 0) {
                return left;
            }
        }
    }
    else if (node->value == ";") {
        CommandResult cmdResult;
        // In the case that the ";" symbol is used after an odd number of commands
        // it should be able the work as a unary operator
        if (!left_result.processed) {
            CommandResult left = execute_command(left_result.output, path);
            if (!right_result.processed) {
                CommandResult right = execute_command(right_result.output, path);
                cmdResult.output = left.output + right.output;
                return  cmdResult;
            }
            else if (right_result.processed) {
                cmdResult.output = left.output + right_result.output;
                return cmdResult;
            }
        }
        else if (left_result.processed) {
            if (!right_result.processed) {
                CommandResult right = execute_command(right_result.output, path);
                cmdResult.output = left_result.output + right.output;
                return cmdResult;
            }
            else if (right_result.processed) {
                cmdResult.output = left_result.output + right_result.output;
                return cmdResult;
            }
        }
    }

    result.output = "This should not be reached.";
    result.exit_status = 1;;
    return result;
}