#include <string>
#include <codecvt>
#include <locale>
#include "helpers.h"

// https://gist.github.com/gchudnov/c1ba72d45e394180e22f
std::string toUtf8(const std::wstring &wide)
{
    std::u16string u16str(wide.begin(), wide.end());
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::string utf8 = convert.to_bytes(u16str);
    return utf8;
}

// Determines if path is relative.
bool isPathRelative(const std::string path)
{
    const auto x = path[0];
    return (x == '.' || x != '/' || x != '\\');
}