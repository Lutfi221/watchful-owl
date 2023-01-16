#include <fstream>
#include <time.h>
#include "config.h"
#include "capturer.h"
#include "helpers.h"
#include "json.hpp"
#include "cppath.hpp"

void appendLogEntry(nlohmann::json entry, Config config, time_t timestamp)
{
    std::string outFilePath;
    char formattedTime[32];
    strftime(formattedTime, sizeof(formattedTime), "%Y%m%d", localtime(&timestamp));
    std::string outFileName = std::string(formattedTime) + ".json.log";

    if (isPathRelative(config.outDir))
        outFilePath = cpppath::normpath(
            cpppath::join({cpppath::curdir(), config.outDir, outFileName}));
    else
        outFilePath = cpppath::join({config.outDir, outFileName});

    std::ofstream outFile;
    outFile.open(outFilePath, std::ios::out | std::ios_base::app);
    if (outFile.fail())
        throw std::ios_base::failure(std::strerror(errno));
    outFile << "\n"
            << entry.dump();
}