cmake_minimum_required(VERSION 3.8)
project(KontrollerService VERSION 1.2.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "Main.cpp" "Service.cpp" "Service.h")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PRIVATE Kontroller)

install(
	TARGETS ${PROJECT_NAME}
	DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/Install"
	PERMISSIONS OWNER_EXECUTE
	CONFIGURATIONS Release
)
