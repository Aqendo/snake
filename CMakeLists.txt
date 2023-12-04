cmake_minimum_required(VERSION 3.13)
project(snake VERSION 0.0.1 LANGUAGES CXX)

add_executable(snake snake.cpp)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
