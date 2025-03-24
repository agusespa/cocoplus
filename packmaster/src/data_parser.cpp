#include "../include/data_parser.h"

#include <sstream>

double DataParser::parseValue(const std::string& key,
                              const std::string& value) {
    try {
        return std::stod(value);
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid number format for key '" + key +
                                    "': " + value);
    }
}

void DataParser::assignValue(Data& data, const std::string& key, double value) {
    if (key == "l") {
        data.left = value;
    } else if (key == "f") {
        data.front = value;
    } else if (key == "r") {
        data.right = value;
    }
}

DataParser::Data DataParser::parse(const std::string& message) {
    Data data;

    std::istringstream iss(message);
    std::string pair;

    while (std::getline(iss, pair, ';')) {
        size_t colon_pos = pair.find(':');
        if (colon_pos == std::string::npos) {
            continue;
        }

        std::string key = pair.substr(0, colon_pos);
        std::string value = pair.substr(colon_pos + 1);

        try {
            double num_value = parseValue(key, value);
            assignValue(data, key, num_value);
        } catch (const std::invalid_argument& e) {
            throw std::invalid_argument(std::string("Failed to parse pair '") +
                                        pair + "': " + e.what());
        }
    }

    return data;
}
