#pragma once
#include <string>

namespace Utils
{
    constexpr int CURVE_KEY_LENGTH = 41;
    std::string trim(const std::string& s);
    std::string GenerateString();
}