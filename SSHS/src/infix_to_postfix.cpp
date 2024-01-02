#include "../include/infix_to_postfix.h"

bool contains_bash_operator(const std::string& str) {
    std::vector<std::string> operators = {"|", "<", ">", "2>", "&&", "||", ";"};

    for (const auto& op : operators) {
        if (str.find(op) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool is_operator_char(char c) {
    return c == '|' || c == '&' || c == '>' || c == '<' || c == ';';
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;
    // unsigned command_nr {0};

    for (size_t i = 0; i < str.length(); ++i) {
        char ch = str[i];

        if (ch == '\'' || ch == '\"') {
            in_quotes = !in_quotes;
        }

        if ((!in_quotes && is_operator_char(ch)) || (ch == '2' && i + 1 < str.size() && str[i + 1] == '>')) {
            if (!token.empty()) {
                tokens.push_back(trim(token));
                // command_nr++;
                token.clear();
            }
            // Handling for the ";" operator
            /*if (ch == ';' && command_nr % 2 != 0) {
                tokens.push_back("");
            }*/
            // Handling for "2>"
            if (ch == '2' && i + 1 < str.size() && str[i + 1] == '>') {
                tokens.push_back("2>");
                ++i;
            } else {
                std::string op(1, ch);
                if (i + 1 < str.size() && is_operator_char(str[i + 1]) && str[i + 1] != '2') {
                    op += str[++i];
                }
                tokens.push_back(op);
            }
        } else {
            token += ch;
        }
    }

    if (!token.empty()) {
        tokens.push_back(trim(token));
    }

    return tokens;
}

bool is_operator(const std::string& token) {
    static const std::unordered_map<std::string, int> operators = {
            {"<", 1}, {"2>", 1},
            {"|", 2},
            {">", 3},
            {"&&", 4}, {"||", 4},
            {";", 5}
    };
    return operators.find(token) != operators.end();
}

std::vector<std::string> convert_to_postfix(const std::vector<std::string>& tokens) {
    std::stack<std::string> stack;
    std::vector<std::string> postfix;
    std::unordered_map<std::string, int> precedence = {
            {"<", 1}, {"2>", 1},
            {"|", 2},
            {">", 3},
            {"&&", 4}, {"||", 4},
            {";", 5}
    };

    for (const std::string& token : tokens) {
        if (!is_operator(token)) {
            postfix.push_back(token);
        } else {
            while (!stack.empty() && precedence[stack.top()] <= precedence[token]) {
                postfix.push_back(stack.top());
                stack.pop();
            }
            stack.push(token);
        }
    }

    while (!stack.empty()) {
        postfix.push_back(stack.top());
        stack.pop();
    }

    return postfix;
}