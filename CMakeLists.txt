cmake_minimum_required(VERSION 3.16)
project(GPTChatbot)

set(CMAKE_CXX_STANDARD 17)

# Tell CMake to include source files
add_executable(gpt_chatbot src/main.cpp)

# Find cpr (you'll use vcpkg to install it)
find_package(cpr CONFIG REQUIRED)

# Link CPR to your binary
target_link_libraries(gpt_chatbot PRIVATE cpr::cpr)
