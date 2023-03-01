#ifndef MAIN_LOGGER
#define MAIN_LOGGER
#include <filesystem>
#include <fstream>
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

        /// @brief Append current symmetric key to file stream encrypted with public key.
        void appendSymKey(std::ofstream *fileStream);
        void generateAndAppendSymKey(std::string logPath);
        void generateAndAppendSymKey(std::ofstream *fileStream);

    public:
        Logger(Config *config);
        ~Logger();

        /// @brief Capture and append a json entry to the log file.
        ///        It also appends the secret AES key
        ///        and rotates it when necessery.
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

    class LogDecryptor
    {
    private:
        crypto::AsymKey *asymKey = nullptr;
        /// @brief Create a SymKey from log data.
        /// @param data Encrypted secret
        /// @param dataLen Length (in bytes) of encrypted secret
        /// @return SymKey
        crypto::SymKey *newSymKeyFromData(CryptoPP::byte *data, size_t dataLen);

    public:
        LogDecryptor(crypto::AsymKey *asymKey);
        ~LogDecryptor();

        void decrypt(CryptoPP::byte *cipher,
                     size_t cipherLen,
                     CryptoPP::byte *plain,
                     size_t plainLen,
                     size_t *outputLen = nullptr);
    };

    void decryptLogFiles(
        std::filesystem::path sourceDir,
        std::filesystem::path destinationDir,
        crypto::AsymKey *asymKey);
}
#endif /* MAIN_LOGGER */
