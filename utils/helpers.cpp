#include "helpers.hpp"

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