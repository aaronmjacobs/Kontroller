cmake_minimum_required(VERSION 3.8)
project(Kontroller-Examples VERSION 0.1.0 LANGUAGES CXX)

add_executable("KontrollerExample-Client" "ClientExample.cpp")
target_link_libraries("KontrollerExample-Client" PRIVATE Kontroller)

add_executable("KontrollerExample-Device" "DeviceExample.cpp")
target_link_libraries("KontrollerExample-Device" PRIVATE Kontroller)

add_executable("KontrollerExample-Pong" "PongExample.cpp")
target_link_libraries("KontrollerExample-Pong" PRIVATE Kontroller)

add_executable("KontrollerExample-Server" "ServerExample.cpp")
target_link_libraries("KontrollerExample-Server" PRIVATE Kontroller)
