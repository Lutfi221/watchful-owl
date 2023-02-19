#include <filesystem>
#include <stdexcept>

#include "spdlog/stopwatch.h"
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/rsa.h>

#include "crypto.h"
#include "dev-logger.h"

crypto::RsaKey::~RsaKey()
{
    delete this->privateKey;
    delete this->publicKey;
}

void crypto::RsaKey::generate(unsigned int size)
{
    using namespace CryptoPP;
    if (this->publicKey != nullptr || this->privateKey != nullptr)
        throw CryptoError("Cannot generate new key on an already populated RsaKey");

    AutoSeededRandomPool rng;
    this->privateKey = new RSA::PrivateKey();
    spdlog::stopwatch sw;
    INFO("Generate private key with size of {} bits", size);
    privateKey->GenerateRandomWithKeySize(rng, size);

    INFO("Generate public key from private key");
    this->publicKey = new RSA::PublicKey(*privateKey);
    INFO("Time taken to generate RSA key pair: `{:.3} seconds`", sw);
}

void crypto::RsaKey::saveToFile(crypto::KeyType keyType, std::string path, std::string password)
{
    CryptoPP::RSAFunction *key = nullptr;
    if (keyType == KeyTypePrivate)
        key = this->privateKey;
    else
        key = this->publicKey;

    if (key == nullptr)
        throw CryptoError("Cannot save a private/public key that is not initialized");

    INFO("Saving {}", keyType == KeyTypePrivate ? "private key" : "public key");
    DEBUG("Copy key to byte queue");
    CryptoPP::ByteQueue q;
    key->Save(q);

    DEBUG("Copy from byte queue to base 64 encoder");
    CryptoPP::Base64Encoder encoder;
    q.CopyTo(encoder);
    encoder.MessageEnd();

    DEBUG("Copy from base 64 encoder to file at `{}`", path);
    CryptoPP::FileSink file(path.c_str());
    encoder.CopyTo(file);
    file.MessageEnd();
}