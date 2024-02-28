cmake_minimum_required(VERSION 3.8)
project(KontrollerService VERSION 1.3.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "Main.cpp" "Service.cpp" "Service.h")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE Kontroller)
target_compile_definitions(${PROJECT_NAME} PRIVATE KONTROLLER_PROJECT_NAME="${KONTROLLER_PROJECT_NAME}" SERVICE_PROJECT_NAME="${PROJECT_NAME}")

install(
	TARGETS ${PROJECT_NAME}
	DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/Install"
	PERMISSIONS OWNER_EXECUTE
	CONFIGURATIONS Release
)
