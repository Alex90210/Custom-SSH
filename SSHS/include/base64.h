#ifndef BASE64_H
#define BASE64_H

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <string>
#include <vector>
#include <iostream>

std::string base64_decode(const std::string& encoded);
std::string base64_encode(const unsigned char* input, size_t length);


#endif