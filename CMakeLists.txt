# Project definition
cmake_minimum_required(VERSION 3.8)
project(Kontroller VERSION 0.1.0 LANGUAGES CXX)

set(LINUX FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(LINUX TRUE)
endif ()

# Build options
option(KONTROLLER_BUILD_EXAMPLES "Build the Kontroller example programs" OFF)

# Library definition and features
add_library(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libraries")
set(QUEUE_DIR "${LIB_DIR}/readerwriterqueue")
target_sources(${PROJECT_NAME} PUBLIC
   "${INC_DIR}/Kontroller/Kontroller.h"
   "${QUEUE_DIR}/atomicops.h"
   "${QUEUE_DIR}/readerwriterqueue.h"
)
target_sources(${PROJECT_NAME} PRIVATE
   "${SRC_DIR}/Communicator.h"
   "${SRC_DIR}/Kontroller.cpp"
)
if (APPLE)
   target_sources(${PROJECT_NAME} PRIVATE
      "${SRC_DIR}/Communicator_osx.cpp"
   )
elseif (WIN32)
   target_sources(${PROJECT_NAME} PRIVATE
      "${SRC_DIR}/Communicator_win.cpp"
   )
elseif (LINUX)
   target_sources(${PROJECT_NAME} PRIVATE
      "${SRC_DIR}/Communicator_linux.cpp"
   )
endif ()
target_include_directories(${PROJECT_NAME}
   PUBLIC "${INC_DIR}" "${QUEUE_DIR}"
   PRIVATE "${SRC_DIR}"
)
get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})

if (KONTROLLER_BUILD_EXAMPLES)
   add_subdirectory("Examples")
endif()

# Platform-specific dependencies
if (APPLE)
   find_library(COREMIDI_LIBRARY CoreMidi)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   target_link_libraries(${PROJECT_NAME} PUBLIC ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif (WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC "winmm")
elseif (LINUX)
endif ()
