cmake_minimum_required(VERSION 3.25.1)
project(SSH_C)

set(CMAKE_CXX_STANDARD 17)

include_directories(/home/alex/json/single_include)

add_executable(SSH_C src/main.cpp
        src/manage_keys.cpp
        src/utils.cpp
        include/utils.h
        include/AES.h
        src/AES.cpp
        include/base64.h
        src/base64.cpp
        include/client.h
        src/client.cpp
        include/read_write.h
        src/read_write.cpp
        include/json_functions.h
        src/json_functions.cpp)

find_package(OpenSSL REQUIRED)

target_link_libraries(SSH_C OpenSSL::SSL OpenSSL::Crypto)
