cmake_minimum_required(VERSION 3.25.1)
project(SSHS)

set(CMAKE_CXX_STANDARD 17)

add_executable(SSHS src/main.cpp
        src/client_handler.cpp
        include/client_handler.h
        include/server.h
        src/server.cpp
        include/json_functions.h
        src/json_functions.cpp
        include/RSA.h
        src/RSA.cpp
        include/base64.h
        src/base64.cpp
        include/AES.h
        src/AES.cpp
        include/exec_command.h
        src/exec_command.cpp
        include/multiple_commands.h
        src/multiple_commands.cpp
        include/infix_to_postfix.h
        src/infix_to_postfix.cpp
        include/command_AST.h
        src/command_AST.cpp
        include/write_read.h
        src/write_read.cpp
        src/experimental.cpp
        include/experimental.h)

# Find the JSON package
find_package(nlohmann_json REQUIRED)

# Find the OpenSSL package
find_package(OpenSSL REQUIRED)

# Link OpenSSL and JSON libraries
target_link_libraries(SSHS OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json)