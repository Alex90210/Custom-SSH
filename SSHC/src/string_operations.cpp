#include "../include/string_operations.h"

bool check_logged(const char* input_string, int size_of_string) {
    if (input_string == nullptr || size_of_string <= 0) {
        return false;
    }

    char* temp = new char[size_of_string];
    int i = 0;
    while (input_string[i] != '\0' && i < size_of_string - 1) {
        temp[i] = std::tolower(static_cast<unsigned char>(input_string[i]));
        i++;
    }
    temp[i] = '\0';

    bool result = strcmp(temp, "login") == 0;

    delete[] temp;
    return result;
}

char* add_len_header(const char* buffer) {

    int string_len = strlen(buffer);
    char *full_message = (char *)malloc(sizeof(int) + string_len);
    if (full_message == NULL) {
        perror("Error allocating memory for full message");
    }

    memcpy(full_message, &string_len, sizeof(int));
    memcpy(full_message + sizeof(int), buffer, string_len);

    return full_message;
}