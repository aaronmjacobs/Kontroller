# Project definition
cmake_minimum_required(VERSION 3.8)
project(Kontroller VERSION 1.4.0 LANGUAGES CXX)

set(LINUX FALSE)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   set(LINUX TRUE)
endif ()

set(KONTROLLER_PROJECT_NAME "${PROJECT_NAME}")

# Build options
option(KONTROLLER_BUILD_SERVICE "Build the Kontroller service" OFF)
option(KONTROLLER_BUILD_EXAMPLES "Build the Kontroller example programs" OFF)

# Library definition and features
add_library(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Libraries")
set(QUEUE_DIR "${LIB_DIR}/readerwriterqueue")
target_sources(${PROJECT_NAME} PRIVATE
   "${INC_DIR}/Kontroller/Client.h"
   "${INC_DIR}/Kontroller/Device.h"
   "${INC_DIR}/Kontroller/Packet.h"
   "${INC_DIR}/Kontroller/Server.h"
   "${INC_DIR}/Kontroller/State.h"
   "${QUEUE_DIR}/atomicops.h"
   "${QUEUE_DIR}/readerwriterqueue.h"
)
target_sources(${PROJECT_NAME} PRIVATE
   "${SRC_DIR}/Client.cpp"
   "${SRC_DIR}/Communicator.h"
   "${SRC_DIR}/Device.cpp"
   "${SRC_DIR}/Server.cpp"
   "${SRC_DIR}/Sock.cpp"
   "${SRC_DIR}/Sock.h"
   "${SRC_DIR}/State.cpp"
)
if (APPLE)
   target_sources(${PROJECT_NAME} PRIVATE
      "${SRC_DIR}/Communicator_macOS.cpp"
   )
elseif (WIN32)
   target_sources(${PROJECT_NAME} PRIVATE
      "${SRC_DIR}/Communicator_Windows.cpp"
   )
elseif (LINUX)
   target_sources(${PROJECT_NAME} PRIVATE
      "${SRC_DIR}/Communicator_Linux.cpp"
   )
endif ()
target_include_directories(${PROJECT_NAME}
   PUBLIC "${INC_DIR}" "${QUEUE_DIR}"
   PRIVATE "${SRC_DIR}"
)
get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCE_FILES})

# PlatformUtils
add_subdirectory("${LIB_DIR}/PlatformUtils")
target_link_libraries(${PROJECT_NAME} PUBLIC PlatformUtils)

# Platform-specific dependencies
if (APPLE)
   find_library(COREMIDI_LIBRARY CoreMidi)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   target_link_libraries(${PROJECT_NAME} PUBLIC ${COREMIDI_LIBRARY} ${COREFOUNDATION_LIBRARY})
elseif (WIN32)
   target_link_libraries(${PROJECT_NAME} PUBLIC "winmm")
elseif (LINUX)
   set(THREADS_PREFER_PTHREAD_FLAG TRUE)
   find_package(Threads REQUIRED)
   target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

   find_package(ALSA REQUIRED)
   target_link_libraries(${PROJECT_NAME} PUBLIC ${ALSA_LIBRARIES})
endif ()

if (KONTROLLER_BUILD_SERVICE)
   add_subdirectory("Service")
endif ()

if (KONTROLLER_BUILD_EXAMPLES)
   add_subdirectory("Examples")
endif()
