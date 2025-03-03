cmake_minimum_required(VERSION 3.14)
project(RobotController)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MOSQUITTO_INCLUDE_DIR "/opt/homebrew/include")
set(MOSQUITTO_LIBRARY "/opt/homebrew/Cellar/mosquitto/2.0.20/lib/libmosquitto.dylib")

find_package(GTest REQUIRED)

include_directories(${MOSQUITTO_INCLUDE_DIR})
link_directories("/opt/homebrew/Cellar/mosquitto/2.0.20/lib")

# Main application
add_executable(robot_controller src/main.cpp src/mqtt_client.cpp)
target_link_libraries(robot_controller ${MOSQUITTO_LIBRARY})

# MQTT client library
add_library(mqtt_library src/mqtt_client.cpp include/mqtt_client.h)
target_include_directories(mqtt_library PUBLIC include)

# MQTT client tests
add_executable(mqtt_client_tests test/mqtt_client_test.cpp)
target_link_libraries(mqtt_client_tests mqtt_library GTest::gtest_main GTest::gmock mosquitto)
target_include_directories(mqtt_client_tests PRIVATE include)

# Main program tests
add_executable(main_tests test/main_test.cpp)
target_link_libraries(main_tests mqtt_library GTest::gtest_main GTest::gmock mosquitto)
target_include_directories(main_tests PRIVATE include)

# Enable testing
enable_testing()
add_test(NAME MqttClientTests COMMAND mqtt_client_tests)
add_test(NAME MainProgramTests COMMAND main_tests)
