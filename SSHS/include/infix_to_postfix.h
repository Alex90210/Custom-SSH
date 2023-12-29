#ifndef INFIX_T0_POSTFIX_H
#define INFIX_T0_POSTFIX_H

#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>

bool isOperatorChar(char c);
std::vector<std::string> tokenize(const std::string& str);
bool isOperator(const std::string& token);
std::vector<std::string> convertToPostfix(const std::vector<std::string>& tokens);
bool containsBashOperator(const std::string& str);

#endif