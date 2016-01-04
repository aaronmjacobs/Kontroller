link_libraries(Kontroller)

add_executable(example Example.cpp)

set_property(TARGET example PROPERTY CXX_STANDARD 11)
set_property(TARGET example PROPERTY CXX_STANDARD_REQUIRED ON)