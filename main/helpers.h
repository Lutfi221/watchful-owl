#ifndef HELPERS
#define HELPERS
#include <string>

std::string toUtf8(const std::wstring &wide);

inline bool fileExists(const std::string &name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
        return false;
}

#endif /* HELPERS */
