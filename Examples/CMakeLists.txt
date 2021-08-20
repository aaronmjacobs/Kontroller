cmake_minimum_required(VERSION 3.8)
project(Kontroller-Examples VERSION 0.1.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "Example.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE Kontroller)
