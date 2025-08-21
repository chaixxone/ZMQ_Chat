#pragma once
#include <string>

namespace Utils
{
    enum ChatMessageFlags
    {
        NoFlags,
        Reply,
        Emotion
    };

    constexpr int CURVE_KEY_LENGTH = 41;
    std::string trim(const std::string& s);
    std::string GenerateString();
}