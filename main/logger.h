#ifndef MAIN_LOGGER
#define MAIN_LOGGER
#include <filesystem>
#include <time.h>

#include "json.hpp"

#include "config.h"
#include "crypto.h"

nlohmann::json generateBasicLogEntry(Config config, time_t timestamp);

namespace logger
{
    enum DataType
    {
        DataTypeJson = 0,
        DataTypeSymKey = 1
    };

    class Logger
    {
    private:
        /// @brief RSA key to encrypt rotating AES keys.
        crypto::AsymKey *asymKey = nullptr;
        /// @brief AES key to encrypt log entries.
        crypto::SymKey *rotatingSymKey = nullptr;
        Config *config = nullptr;

        /// @brief How many logs since the last AES key generation.
        unsigned int logsSinceLatestKeyGen = 0;
        /// @brief The date (in YYYYMMDD format) of the last append.
        std::string lastAppendDate;
        /// @brief Path to the log directory.
        std::filesystem::path outDir;

        /// @brief Get the appropriate log file name. If encryption is enabled,
        ///        will create a new encrypted log file for the day (if it doesn't exist)
        ///        and put a version specifier on the first byte.
        /// @param timestamp Unix timestamp
        /// @return Full log file path
        std::string prepareLogFile(time_t timestamp);
        void appendBinary(DataType type, unsigned char *data, size_t dataLen, std::ofstream *fileStream);

    public:
        Logger(Config *config);
        ~Logger();

        void captureAndAppend();
        /// @brief Capture a log snapshot.
        /// @param timestamp Current time in UNIX
        /// @param durationSinceLastInput Seconds since last user input or interaction
        /// @return JSON log entry
        nlohmann::json capture(time_t timestamp, unsigned int durationSinceLastInput = 0);

        /// @brief Append a json entry to the log file.
        /// @param entry JSON entry
        /// @param logPath Log path
        /// @param encryptedBinary Should it be encrypted?
        void append(nlohmann::json entry, std::string logPath, bool encryptedBinary = false);
    };
}
#endif /* MAIN_LOGGER */
