cmake_minimum_required(VERSION 3.8)
project(Kontroller-Examples VERSION 0.1.0 LANGUAGES CXX)

add_executable("Kontroller-ClientExample" "ClientExample.cpp")
target_link_libraries("Kontroller-ClientExample" PRIVATE Kontroller)

add_executable("Kontroller-DeviceExample" "DeviceExample.cpp")
target_link_libraries("Kontroller-DeviceExample" PRIVATE Kontroller)

add_executable("Kontroller-ServerExample" "ServerExample.cpp")
target_link_libraries("Kontroller-ServerExample" PRIVATE Kontroller)
