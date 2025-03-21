cmake_minimum_required(VERSION 3.28)
project(RobotController LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Mosquitto paths
set(MOSQUITTO_INCLUDE_DIR "/opt/homebrew/include")
set(MOSQUITTO_LIBRARY "/opt/homebrew/Cellar/mosquitto/2.0.21/lib/libmosquitto.dylib")

# Fetch SFML 3.0
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

find_package(Threads REQUIRED)

# Include directories
include_directories(${MOSQUITTO_INCLUDE_DIR})
include_directories(include)
link_directories("/opt/homebrew/Cellar/mosquitto/2.0.21/lib")

# Source files for the robot controller
set(ROBOT_CONTROLLER_SOURCES
    src/main.cpp
    src/controller.cpp
    src/monitor.cpp
    src/robot_view.cpp
    src/health_tracker.cpp
    src/mqtt_client.cpp
)

# MQTT client library
add_library(mqtt_library 
    src/mqtt_client.cpp 
    include/mqtt_client.h
)
target_include_directories(mqtt_library PUBLIC include)
target_link_libraries(mqtt_library ${MOSQUITTO_LIBRARY})

# Main application
add_executable(robot_controller ${ROBOT_CONTROLLER_SOURCES})
target_link_libraries(robot_controller 
    mqtt_library
    SFML::Graphics
    SFML::Window
    SFML::System
    Threads::Threads
)
