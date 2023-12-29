#include "../include/command_AST.h"

struct TreeNode {
    std::string value;
    TreeNode *left, *right;

    explicit TreeNode(std::string val) : value(std::move(val)), left(nullptr), right(nullptr) {}
};

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