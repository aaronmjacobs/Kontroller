# Project definition
cmake_minimum_required(VERSION 3.1)
project(Kontroller VERSION 0.1.0 LANGUAGES CXX)

set(LINUX FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(LINUX TRUE)
endif ()

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(QUEUE_DIR "${LIB_DIR}/readerwriterqueue")
set(SOURCES)
list(APPEND SOURCES
   "${SRC_DIR}/Kontroller.cpp"
   "${SRC_DIR}/Communicator.h"
   "${INC_DIR}/Kontroller/Kontroller.h"
   "${QUEUE_DIR}/atomicops.h"
   "${QUEUE_DIR}/readerwriterqueue.h"
)
if (APPLE)
   list(APPEND SOURCES
      "${SRC_DIR}/Communicator_osx.cpp"
   )
elseif (WIN32)
   list(APPEND SOURCES
      "${SRC_DIR}/Communicator_win.cpp"
   )
elseif (LINUX)
   list(APPEND SOURCES
      "${SRC_DIR}/Communicator_linux.cpp"
   )
endif ()

# Library definition and features
add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME}
   PUBLIC "${INC_DIR}" "${QUEUE_DIR}"
   PRIVATE "${SRC_DIR}"
)
set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED ON
)

# Platform-specific dependencies
if (APPLE)
   find_library(COREMIDI_LIBRARY CoreMidi)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   target_link_libraries(${PROJECT_NAME} PUBLIC ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif (WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC "winmm")
elseif (LINUX)
endif ()

# Build options
option(KONTROLLER_BUILD_EXAMPLES "Build the Kontroller example programs" OFF)
if (KONTROLLER_BUILD_EXAMPLES)
   add_subdirectory("examples")
endif()