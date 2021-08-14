# Project definition
cmake_minimum_required(VERSION 3.1)
project(Kontroller-Examples VERSION 0.1.0 LANGUAGES CXX)

# Executable definition and features
add_executable(${PROJECT_NAME} "Example.cpp")
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:Kontroller,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${PROJECT_NAME} PRIVATE Kontroller $<TARGET_PROPERTY:Kontroller,INTERFACE_LINK_LIBRARIES>)
target_compile_definitions(${PROJECT_NAME} INTERFACE $<TARGET_PROPERTY:Kontroller,INTERFACE_COMPILE_DEFINITIONS>)
set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED ON
)