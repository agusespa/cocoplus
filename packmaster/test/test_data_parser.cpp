#include <gtest/gtest.h>

#include "data_parser.h"

TEST(DataParserTest, ParsesBasicMessage) {
    std::string message = "l:1.5;f:2.0;r:0.5";
    auto result = DataParser::parse(message);

    EXPECT_DOUBLE_EQ(result.left, 1.5);
    EXPECT_DOUBLE_EQ(result.front, 2.0);
    EXPECT_DOUBLE_EQ(result.right, 0.5);
}

TEST(DataParserTest, HandlesNegativeNumbers) {
    std::string message = "l:-1.5;f:-2.0;r:-0.5";
    auto result = DataParser::parse(message);

    EXPECT_DOUBLE_EQ(result.left, -1.5);
    EXPECT_DOUBLE_EQ(result.front, -2.0);
    EXPECT_DOUBLE_EQ(result.right, -0.5);
}

TEST(DataParserTest, HandlesMissingValues) {
    std::string message = "f:2.5";
    auto result = DataParser::parse(message);

    EXPECT_DOUBLE_EQ(result.front, 2.5);
    EXPECT_DOUBLE_EQ(result.left, 0.0);   // Default value
    EXPECT_DOUBLE_EQ(result.right, 0.0);  // Default value
}

TEST(DataParserTest, ThrowsOnInvalidNumberFormat) {
    std::string message = "l:abc;f:2.0";
    EXPECT_THROW(DataParser::parse(message), std::invalid_argument);
}

TEST(DataParserTest, HandlesScientificNotation) {
    std::string message = "l:1.5e-1;f:2.0E2";
    auto result = DataParser::parse(message);

    EXPECT_DOUBLE_EQ(result.left, 0.15);
    EXPECT_DOUBLE_EQ(result.front, 200.0);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
