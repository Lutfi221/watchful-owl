#ifndef MAIN_LOGGER
#define MAIN_LOGGER

#include <time.h>
#include "config.h"
#include "json.hpp"

void appendLogEntry(nlohmann::json entry, Config config, time_t timestamp);

#endif /* MAIN_LOGGER */
