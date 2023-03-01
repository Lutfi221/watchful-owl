#include <filesystem>
#include <fstream>
#include <map>
#include <time.h>

#include "capturer.h"
#include "config.h"
#include "dev-logger.h"
#include "helpers.h"
#include "json.hpp"
#include "logger.h"

#define ENC_LOGFILE_VERSION 'A'
#define ENC_LOGFILE_SUFFIX ".json.log.enc"
#define LOGFILE_SUFFIX ".json.log"
#define ENC_LOGFILE_REGEX_PATTERN "\\d{8}\\.json\\.log\\.enc"
#define LOGFILE_BASE_NAME_PATTERN "\\d{8}"

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define LITTLE_ENDIAN 1
#else
#define LITTLE_ENDIAN 0
#endif

const std::map<unsigned char, logger::DataType> BYTE_TO_DATA_TYPE{
    {0, logger::DataTypeJson},
    {1, logger::DataTypeSymKey}};

/// @brief Capture a snapshot
/// @param timestamp UNIX timestamp
/// @return Snapshot
nlohmann::json generateBasicLogEntry(time_t timestamp)
{
    nlohmann::json entry;
    entry["time"] = timestamp;

    std::vector<AppRecord> apps;
    getOpenedApps(&apps);

    entry["apps"] = nlohmann::json::array();

    for (auto const &appRecord : apps)
    {
        entry["apps"].push_back({{"title", appRecord.title},
                                 {"path", appRecord.path}});
        if (appRecord.isActive)
            entry["apps"].back()["isActive"] = true;
    };

    return entry;
}

logger::Logger::Logger(Config *config)
{
    this->config = config;
    this->outDir = prepareAndProcessPath(config->outDir, true, true);

    if (config->encryption.enabled)
    {
        this->asymKey = new crypto::AsymKey();
        auto publicKeyPath = prepareAndProcessPath(config->encryption.rsaPublicKeyPath);
        this->asymKey->loadFromFile(crypto::KeyTypePublic, publicKeyPath.u8string());

        if (!this->asymKey->validate(crypto::KeyTypePublic))
            throw crypto::CryptoError("Invalid public key");
    }
};

void logger::Logger::captureAndAppend()
{
    time_t timestamp = time(nullptr);
    char outBuffer[32];
    std::string logPath = this->prepareLogFile(timestamp);

    auto logEntry = this->capture(timestamp, getDurationSinceLastInput());

    if (this->config->encryption.enabled)
    {
        if (this->rotatingSymKey == nullptr)
            this->generateAndAppendSymKey(logPath);
        else if (this->logsSinceLatestKeyGen >= this->config->encryption.keyGenRate)
        {
            this->generateAndAppendSymKey(logPath);
            this->logsSinceLatestKeyGen = 0;
        }
        else
            this->logsSinceLatestKeyGen++;
    }

    this->append(logEntry, logPath, this->config->encryption.enabled);
}

nlohmann::json logger::Logger::capture(time_t timestamp, unsigned int durationSinceLastInput)
{
    nlohmann::json entry;

    if (durationSinceLastInput > this->config->idleThreshold)
    {
        DEBUG("User is away. Last input is {} seconds ago.", durationSinceLastInput);
        entry["timestamp"] = timestamp;
        entry["durationSinceLastInput"] = durationSinceLastInput;
    }
    else
        entry = generateBasicLogEntry(timestamp);

    return entry;
}

void logger::Logger::append(nlohmann::json entry, std::string logPath, bool encryptedBinary)
{
    if (!encryptedBinary)
    {
        std::ofstream logFile(logPath, std::ios::out | std::ios_base::app);
        logFile << "\n"
                << entry.dump();
    }

    std::ofstream logFile(logPath, std::ios::binary | std::ios::out | std::ios_base::app);

    assert(this->rotatingSymKey != nullptr);

    std::string jsonStr(entry.dump());

    size_t cipherLen = this->rotatingSymKey->calculateCipherLen(jsonStr.size());
    unsigned char cipher[cipherLen];

    this->rotatingSymKey->encrypt(
        (unsigned char *)jsonStr.c_str(),
        jsonStr.size(),
        &cipher[0],
        cipherLen);

    this->appendBinary(DataTypeJson, &cipher[0], cipherLen, &logFile);
}

void logger::Logger::appendBinary(logger::DataType type, unsigned char *data,
                                  size_t dataLen, std::ofstream *fileStream)
{
    if (dataLen > 16777215)
        throw std::runtime_error("Data exceeds supported length of 16 megabytes");

    unsigned char entryLen[3];

#if LITTLE_ENDIAN == 1

    unsigned char *src = (unsigned char *)&dataLen;
    unsigned char *dst = &entryLen[0];

    // If we assume the `sizeof entryLen` is 3,
    // then  `i` will go {0, 1, 2},
    // while `j` will go {2, 1, 0}.
    for (int i = 0; i < sizeof entryLen; i++)
    {
        int j = sizeof entryLen - i - 1;
        memcpy(dst + j, src + i, 1);
    }
#else
    memcpy(&entryLen, &dataLen, sizeof entryLen);
#endif

    fileStream->write((char *)&type, 1);
    fileStream->write((char *)entryLen, sizeof entryLen);
    fileStream->write((char *)data, dataLen);
};

std::string logger::Logger::prepareLogFile(time_t timestamp)
{
    char outBuffer[10];
    strftime(outBuffer, sizeof(outBuffer), "%Y%m%d", localtime(&timestamp));
    std::string date(outBuffer);
    DEBUG("Prepare log file at timestamp {} and date {}", timestamp, date);

    if (!config->encryption.enabled)
        return (this->outDir / std::filesystem::path(date + LOGFILE_SUFFIX)).u8string();

    auto logPath = this->outDir / std::filesystem::path(date + ENC_LOGFILE_SUFFIX);
    if (std::filesystem::exists(logPath))
    {
        DEBUG("Encrypted log file for the day already exists.");
        return logPath.u8string();
    }

    DEBUG("Create encrypted log file for the day and "
          "put a version specifier on the first byte.");
    this->lastAppendDate = date;
    std::ofstream f(logPath, std::ios::binary | std::ios::out | std::ios_base::app);

    char logfileVersion = ENC_LOGFILE_VERSION;
    f.write(&logfileVersion, 1);

    return logPath.u8string();
}

void logger::Logger::generateAndAppendSymKey(std::ofstream *fileStream)
{
    delete this->rotatingSymKey;
    this->rotatingSymKey = new crypto::SymKey();
    this->rotatingSymKey->generateRandom();
    this->appendSymKey(fileStream);
}
void logger::Logger::generateAndAppendSymKey(std::string logPath)
{
    std::ofstream f(logPath, std::ios::binary | std::ios::out | std::ios_base::app);
    this->generateAndAppendSymKey(&f);
}

void logger::Logger::appendSymKey(std::ofstream *fileStream)
{
    size_t secretLen = this->rotatingSymKey->getSecretLen();
    unsigned char secret[secretLen];
    this->rotatingSymKey->getSecret(secret, secretLen);

    size_t cipherLen = this->asymKey->calculateCipherLen();
    unsigned char cipher[cipherLen];

    this->asymKey->encrypt(secret, secretLen, &cipher[0], cipherLen);

    this->appendBinary(DataTypeSymKey, &cipher[0], cipherLen, fileStream);
}

logger::Logger::~Logger()
{
    delete this->asymKey;
    delete this->rotatingSymKey;
}

logger::LogDecryptor::LogDecryptor(crypto::AsymKey *asymKey) : asymKey(asymKey) {}

void logger::LogDecryptor::decrypt(CryptoPP::byte *cipher,
                                   size_t cipherLen,
                                   CryptoPP::byte *plain,
                                   size_t plainLen,
                                   size_t *outputLen)
{
    assert(plainLen >= cipherLen);

    CryptoPP::byte *pCipher = cipher;
    CryptoPP::byte *pPlain = plain;

    CryptoPP::byte *pCipherEnd = pCipher + cipherLen - 1;
    crypto::SymKey *rotatingSymKey = nullptr;

    char versionSpecifier = *pCipher;
    pCipher++;

    if (versionSpecifier != 'A')
        throw std::runtime_error("Invalid version specifier in log data.");

    logger::DataType dataType;
    unsigned long int dataLen = 0;

    while (pCipher < pCipherEnd)
    {
        dataType = BYTE_TO_DATA_TYPE.at(*pCipher);
        pCipher++;

        dataLen = (pCipher[2] << 0) | (pCipher[1] << 8) | (pCipher[0] << 16);
        pCipher += 3;

        if (dataType == logger::DataTypeSymKey)
        {
            delete rotatingSymKey;
            rotatingSymKey = this->newSymKeyFromData(pCipher, dataLen);
        }
        else
        {
            *pPlain = '\n';
            pPlain++;

            size_t outputLen = 0;
            rotatingSymKey->decrypt(pCipher, dataLen, pPlain, dataLen, &outputLen);
            pPlain += outputLen;
        }

        pCipher += dataLen;
    }

    *outputLen = pPlain - plain;
    delete rotatingSymKey;
}

crypto::SymKey *logger::LogDecryptor::newSymKeyFromData(CryptoPP::byte *data, size_t dataLen)
{
    CryptoPP::byte outBuffer[dataLen];
    size_t outputLen = 0;

    this->asymKey->decrypt(data, dataLen, outBuffer, dataLen, &outputLen);
    return new crypto::SymKey(outBuffer, outputLen);
}

logger::LogDecryptor::~LogDecryptor()
{
}

void logger::decryptLogFiles(
    std::filesystem::path sourceDir,
    std::filesystem::path destinationDir,
    crypto::AsymKey *asymKey)
{
    using namespace std;
    LogDecryptor logDecryptor(asymKey);
    regex baseNamePattern(LOGFILE_BASE_NAME_PATTERN);

    vector<filesystem::path> files = getFileListByRegex(
        sourceDir,
        regex(ENC_LOGFILE_REGEX_PATTERN,
              regex_constants::icase));
    INFO("Found {} log files to decrypt", files.size());

    for (auto &file : files)
    {
        auto size = filesystem::file_size(file);
        INFO("Process log file `{}` with size of {} bytes", file.string(), size);
        unsigned char inBuffer[size];
        unsigned char outBuffer[size];
        size_t outputLen = 0;

        DEBUG("Read log file to buffer");
        ifstream f(file, ios::binary);
        f.read((char *)&inBuffer[0], size);

        DEBUG("Decrypt log file in buffer");
        logDecryptor.decrypt(&inBuffer[0], size, &outBuffer[0], size, &outputLen);

        auto fileName = file.filename().u8string();
        smatch baseNameMatch;
        regex_search(fileName, baseNameMatch, baseNamePattern);
        string outputFileName = string(baseNameMatch[0]) + string(LOGFILE_SUFFIX);

        ofstream of(destinationDir / filesystem::path(outputFileName), ios::binary | ios::app);
        of.write((char *)&outBuffer[0], outputLen);
    }
}
