#ifndef MAIN_CONSTANTS
#define MAIN_CONSTANTS
#include <filesystem>
#include <string>

#include "helpers.h"

namespace constants
{
    const std::string PERPETUAL_EXE_FILENAME = std::string(PERPETUAL_TARGET_NAME) + u8".exe";
    const std::filesystem::path LOG_OUTPUT_DIR = getExecutableDirPath() /
                                                 std::filesystem::path("./dev-logs/");
}

#endif /* MAIN_CONSTANTS */
