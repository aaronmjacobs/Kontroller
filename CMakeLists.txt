# Project definition
cmake_minimum_required(VERSION 3.1)
project(Kontroller VERSION 0.1.0 LANGUAGES CXX)

# Source files
set(KONTROLLER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(KONTROLLER_SOURCES)
set(KONTROLLER_LINUX ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
list(APPEND KONTROLLER_SOURCES
   "${KONTROLLER_SRC_DIR}/Kontroller.cpp"
)
if (APPLE)
   list(APPEND KONTROLLER_SOURCES
      "${KONTROLLER_SRC_DIR}/Communicator_osx.cpp"
   )
elseif (WIN32)
   list(APPEND KONTROLLER_SOURCES
      "${KONTROLLER_SRC_DIR}/Communicator_win.cpp"
   )
elseif (KONTROLLER_LINUX)
   list(APPEND KONTROLLER_SOURCES
      "${KONTROLLER_SRC_DIR}/Communicator_linux.cpp"
   )
endif (APPLE)

# Library definition and features
add_library(${PROJECT_NAME} ${KONTROLLER_SOURCES})
target_include_directories(${PROJECT_NAME}
   PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
   PRIVATE "${KONTROLLER_SRC_DIR}"
)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_strong_enums cxx_nullptr)

# Platform-specific dependencies
if (APPLE)
   find_library(COREMIDI_LIBRARY CoreMidi)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   target_link_libraries(${PROJECT_NAME} PUBLIC ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif (WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC "winmm")
elseif (KONTROLLER_LINUX)
endif (APPLE)

# Build options
option(KONTROLLER_BUILD_EXAMPLES "Build the Kontroller example programs" OFF)
if (KONTROLLER_BUILD_EXAMPLES)
   add_subdirectory("examples")
endif(KONTROLLER_BUILD_EXAMPLES)