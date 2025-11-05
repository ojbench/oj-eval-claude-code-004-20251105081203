cmake_minimum_required(VERSION 3.10)
project(BookstoreManagementSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Create the executable
add_executable(code main.cpp)

# Add any additional source files here if needed
# target_sources(code PRIVATE other_file.cpp)