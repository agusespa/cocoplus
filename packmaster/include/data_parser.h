#pragma once
#include <string>

class DataParser {
   public:
    struct Data {
        double left = 0.0;
        double front = 0.0;
        double right = 0.0;
    };

    static Data parse(const std::string& message);

   private:
    static double parseValue(const std::string& key, const std::string& value);
    static void assignValue(Data& data, const std::string& key, double value);
};
