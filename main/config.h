#ifndef MAIN_CONFIG
#define MAIN_CONFIG
#include <string>

struct Config
{
    std::string outDir;
};

Config loadConfig(bool createIfMissing = 0);

#endif /* MAIN_CONFIG */
