#ifndef INFIX_T0_POSTFIX_H
#define INFIX_T0_POSTFIX_H

#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

bool is_operator_char(char c);
bool is_operator(const std::string& token);
std::vector<std::string> tokenize(const std::string& str);
std::vector<std::string> convert_to_postfix(const std::vector<std::string>& tokens);
bool contains_bash_operator(const std::string& str);

#endif