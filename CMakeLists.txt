cmake_minimum_required(VERSION 3.10)
project(QuizGameCPP VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBWEBSOCKETS REQUIRED libwebsockets)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/cpp-server
    ${LIBWEBSOCKETS_INCLUDE_DIRS}
)

# Game Server executable
add_executable(game_server
    cpp-server/main.cpp
    cpp-server/game_logic.cpp
    cpp-server/json_loader.cpp
)

target_link_libraries(game_server
    ${LIBWEBSOCKETS_LIBRARIES}
    pthread
)

# HTTP Server executable
add_executable(http_server
    cpp-server/http_server.cpp
)

target_link_libraries(http_server
    ${LIBWEBSOCKETS_LIBRARIES}
)

# Copy resources to build directory
add_custom_command(TARGET game_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/websocket-bridge/questions
        ${CMAKE_BINARY_DIR}/questions
    COMMENT "Copying question files..."
)

add_custom_command(TARGET http_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/public
        ${CMAKE_BINARY_DIR}/public
    COMMENT "Copying public web files..."
)

# Installation
install(TARGETS game_server http_server
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/public/
    DESTINATION share/quizgame/public
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/websocket-bridge/questions/
    DESTINATION share/quizgame/questions
)

# Print build information
message(STATUS "=================================")
message(STATUS "Quiz Game C++ Server Build")
message(STATUS "=================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source Dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "Binary Dir: ${CMAKE_BINARY_DIR}")
message(STATUS "=================================")
