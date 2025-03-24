cmake_minimum_required(VERSION 3.14)
project(data_parser_test CXX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
FetchContent_MakeAvailable(googletest)

include(CTest)

add_executable(data_parser_test
    test_data_parser.cpp
)

target_link_libraries(data_parser_test
  PRIVATE
  GTest::gtest_main
  data_parser
)

add_test(NAME data_parser_test
    COMMAND data_parser_test
)

enable_testing()
