cmake_minimum_required(VERSION 3.8)
project(Kontroller-Examples VERSION 0.1.0 LANGUAGES CXX)

include_directories( ${CMAKE_SOURCE_DIR}/../Include )
include_directories( ${CMAKE_SOURCE_DIR}/../Include/Kontroller )
include_directories( ${CMAKE_SOURCE_DIR}/../Libraries/readerwriterqueue )

add_executable("KontrollerExample-Client" "ClientExample.cpp")
target_compile_features("KontrollerExample-Client" PRIVATE cxx_std_17)
target_link_libraries("KontrollerExample-Client" PRIVATE Kontroller)

add_executable("KontrollerExample-Device" "DeviceExample.cpp")
target_compile_features("KontrollerExample-Device" PRIVATE cxx_std_17)
target_link_libraries("KontrollerExample-Device" PRIVATE Kontroller)

add_executable("KontrollerExample-Pong" "PongExample.cpp")
target_compile_features("KontrollerExample-Pong" PRIVATE cxx_std_17)
target_link_libraries("KontrollerExample-Pong" PRIVATE Kontroller)

add_executable("KontrollerExample-Server" "ServerExample.cpp")
target_compile_features("KontrollerExample-Server" PRIVATE cxx_std_17)
target_link_libraries("KontrollerExample-Server" PRIVATE Kontroller)
