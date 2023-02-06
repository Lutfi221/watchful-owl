#ifndef MAIN_LOGGER
#define MAIN_LOGGER

#include <time.h>
#include "config.h"
#include "json.hpp"

nlohmann::json generateBasicLogEntry(Config config, time_t timestamp);
void captureAndAppend(Config config);
void appendLogEntry(nlohmann::json entry, Config config, time_t timestamp);

#endif /* MAIN_LOGGER */
