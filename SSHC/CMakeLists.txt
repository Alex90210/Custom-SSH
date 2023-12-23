cmake_minimum_required(VERSION 3.25.1)
project(SSH_C)

set(CMAKE_CXX_STANDARD 17)

include_directories(/home/alex/json/single_include)

add_executable(SSH_C src/main.cpp
        src/keys.cpp
        src/string_operations.cpp
        include/string_operations.h
        src/json_functions.cpp
        include/json_functions.h
        src/client_functions.cpp
        include/client_functions.h
        include/AES.h
        src/AES.cpp
        include/base64.h
        src/base64.cpp)

find_package(OpenSSL REQUIRED)

target_link_libraries(SSH_C OpenSSL::SSL OpenSSL::Crypto)
