#include "helpers.hpp"
#include <sodium.h>

std::string Utils::trim(const std::string& s)
{
    size_t lindex = 0;
    size_t size = s.size();

    while (lindex < size && s[lindex] == ' ')
    {
        lindex++;
    }

    size_t rindex = lindex;

    while (rindex < size && s[rindex] != ' ')
    {
        rindex++;
    }

    return std::string(s.begin() + lindex, s.begin() + rindex);
}

std::string Utils::GenerateString()
{
    constexpr int BINARY_BUFFER_SIZE = 16;
    constexpr int HEX_BUFFER_SIZE = BINARY_BUFFER_SIZE * 2 + 1;

    unsigned char binaryBuffer[BINARY_BUFFER_SIZE];
    // Generate binary representation of string
    randombytes_buf(binaryBuffer, BINARY_BUFFER_SIZE);

    char hexBuffer[HEX_BUFFER_SIZE];
    sodium_bin2hex(hexBuffer, HEX_BUFFER_SIZE, binaryBuffer, BINARY_BUFFER_SIZE);

    return hexBuffer;
}