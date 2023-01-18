#ifndef MAIN_HELPERS
#define MAIN_HELPERS
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

bool isPathRelative(const std::string path);

std::string getExecutablePath();

#endif /* MAIN_HELPERS */
