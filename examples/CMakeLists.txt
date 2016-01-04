link_libraries(Kontroller)

add_executable(Example Example.cpp)

set_property(TARGET Example PROPERTY CXX_STANDARD 11)
set_property(TARGET Example PROPERTY CXX_STANDARD_REQUIRED ON)