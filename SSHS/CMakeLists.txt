cmake_minimum_required(VERSION 3.25.1)
project(SSHS)

set(CMAKE_CXX_STANDARD 17)

# Find the JSON package
find_package(nlohmann_json REQUIRED)

add_executable(SSHS src/main.cpp
        src/client_handler.cpp
        include/client_handler.h
        include/server_functions.h
        src/server_functions.cpp
        include/json_functions.h
        src/json_functions.cpp
        include/rsa_key.h
        src/rsa_key.cpp
        include/base64.h
        src/base64.cpp
        include/AES.h
        src/AES.cpp)

# Find the OpenSSL package
find_package(OpenSSL REQUIRED)

# Link OpenSSL and JSON libraries
target_link_libraries(SSHS OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json)
