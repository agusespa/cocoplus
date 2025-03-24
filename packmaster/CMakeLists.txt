cmake_minimum_required(VERSION 3.16)
project(RobotController LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(MOSQUITTO_INCLUDE_DIR "/opt/homebrew/include")
set(MOSQUITTO_LIBRARY "/opt/homebrew/Cellar/mosquitto/2.0.21/lib/libmosquitto.dylib")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

find_package(Threads REQUIRED)

include_directories(${MOSQUITTO_INCLUDE_DIR})
include_directories(include)
link_directories("/opt/homebrew/Cellar/mosquitto/2.0.21/lib")

set(ROBOT_CONTROLLER_SOURCES
    src/main.cpp
    src/controller.cpp
    src/monitor.cpp
    src/robot_view.cpp
    src/health_tracker.cpp
    src/mqtt_client.cpp
    src/data_parser.cpp
)

add_library(mqtt_library 
    src/mqtt_client.cpp 
)
target_include_directories(mqtt_library PUBLIC include)
target_link_libraries(mqtt_library ${MOSQUITTO_LIBRARY})

add_library(data_parser
    src/data_parser.cpp
)
target_include_directories(data_parser PUBLIC include)

add_executable(robot_controller ${ROBOT_CONTROLLER_SOURCES})
target_link_libraries(robot_controller 
    mqtt_library
    Threads::Threads
    SDL3::SDL3
)

add_subdirectory(test)
