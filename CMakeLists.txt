cmake_minimum_required(VERSION 3.1)
project(Kontroller)

set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(SOURCES)
set(INCLUDES)
set(LIBRARIES)

list(APPEND SOURCES
   "${SRC_DIR}/Kontroller.cpp"
)

list(APPEND INCLUDES
   "${INC_DIR}/Kontroller/Kontroller.h"
)

if(APPLE)
   list(APPEND SOURCES
      "${SRC_DIR}/Communicator_osx.cpp"
   )
   find_library(COREMIDI_LIBRARY CoreMidi)
   list(APPEND LIBRARIES
      ${COREMIDI_LIBRARY}
   )
endif(APPLE)

if(WIN32)
   list(APPEND SOURCES
      "${SRC_DIR}/Communicator_win.cpp"
   )
   list(APPEND LIBRARIES
      "winmm"
   )
endif(WIN32)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   list(APPEND SOURCES
      "${SRC_DIR}/Communicator_linux.cpp"
   )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(${INC_DIR})
add_library(${PROJECT_NAME} ${SOURCES} ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${LIBRARIES})

# Compiler requirements
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

if(BUILD_SHARED_LIBS)
   list(APPEND LIBRARIES
      ${PROJECT_NAME}
   )
endif(BUILD_SHARED_LIBS)

# Export
set(KONTROLLER_TARGET ${PROJECT_NAME} PARENT_SCOPE)
set(KONTROLLER_LIBRARIES ${LIBRARIES} PARENT_SCOPE)
set(KONTROLLER_INCLUDE_DIR ${INC_DIR} PARENT_SCOPE)