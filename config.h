#ifndef CONFIG
#define CONFIG
#include <string>

struct Config {
    std::string outDir;
};

Config loadConfig(bool createIfMissing = 0);


#endif /* CONFIG */
