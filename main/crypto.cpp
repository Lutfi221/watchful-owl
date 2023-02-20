#include <filesystem>
#include <stdexcept>

#include "spdlog/stopwatch.h"
#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/ccm.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/files.h>
#include <cryptopp/osrng.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/rsa.h>

#include "crypto.h"
#include "dev-logger.h"

/// AES key length in bytes
#define AES_KEY_LEN 16
#define AES_BLOCKSIZE CryptoPP::AES::BLOCKSIZE
/// Salt length in bytes
#define SALT_LEN 16
#define PBKDF2_ITERATIONS 1024

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
    using namespace CryptoPP;
    FileSink file(path.c_str());
    ByteQueue q;
    RSAFunction *key = nullptr;

    if (keyType == KeyTypePrivate)
        key = this->privateKey;
    else
        key = this->publicKey;

    if (key == nullptr)
        throw CryptoError("Cannot save a private/public key that is not initialized");

    INFO("Saving {}", keyType == KeyTypePrivate ? "private key" : "public key");
    DEBUG("Copy key to byte queue");
    key->Save(q);

    if (!password.empty())
    {
        auto plainLen = q.CurrentSize();
        // TODO: Use secure bytes from CryptoPP
        DEBUG("Copy key to a byte array for encryption");
        byte *plain = new byte[plainLen];
        q.Get(plain, plainLen);

        auto cipherLen = plainLen + 2 * AES_BLOCKSIZE;
        DEBUG("plainLen: `{} bytes`, cipherLen: `{} bytes`", plainLen, cipherLen);
        byte *cipher = new byte[cipherLen];

        DEBUG("Generate random salt");
        AutoSeededRandomPool prng;
        byte *salt = new byte[SALT_LEN];
        prng.GenerateBlock(salt, SALT_LEN);

        DEBUG("Initialize SymmetricKey");
        auto symKey = SymmetricKey(password, salt, SALT_LEN);
        INFO("Encrypt key");
        symKey.encrypt(plain, plainLen, cipher, cipherLen);

        DEBUG("Clear byte queue");
        q.Clear();
        DEBUG("Put encrypted key into byte queue");
        q.Put(cipher, cipherLen);

        DEBUG("Convert salt to base64 and put into file sink");
        Base64Encoder encoder;
        encoder.Put(salt, SALT_LEN);
        encoder.CopyTo(file);
        encoder.MessageEnd();
        file.Put('\n');
        file.Put('\n');

        delete[] plain;
        delete[] cipher;
        delete[] salt;
    };

    DEBUG("Copy from byte queue to base 64 encoder");
    Base64Encoder encoder;
    q.CopyTo(encoder);
    encoder.MessageEnd();

    DEBUG("Copy from base 64 encoder to file at `{}`", path);
    encoder.CopyTo(file);

    file.MessageEnd();
}

void derivePassword(CryptoPP::byte *derived,
                    size_t derivedLen,
                    const CryptoPP::byte *password,
                    size_t passwordLen,
                    const CryptoPP::byte *salt,
                    size_t saltLen,
                    unsigned int iterations = PBKDF2_ITERATIONS)
{
    using namespace CryptoPP;
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
    byte unused = 0;
    pbkdf.DeriveKey(derived, derivedLen,
                    unused, password, passwordLen,
                    salt, saltLen, iterations);
}

crypto::SymmetricKey::SymmetricKey(std::string password, CryptoPP::byte *salt, size_t saltLen)
    : secretLen(AES_KEY_LEN)
{
    CryptoPP::byte *p = new CryptoPP::byte[password.size()];
    std::copy(password.begin(), password.end(), p);

    this->secret = new CryptoPP::byte[AES_KEY_LEN];
    derivePassword(this->secret, this->secretLen, p, sizeof(p), salt, saltLen);

    delete[] p;
}
crypto::SymmetricKey::~SymmetricKey()
{
    delete[] this->secret;
}

void crypto::SymmetricKey::encrypt(
    CryptoPP::byte *plain, size_t plainLen,
    CryptoPP::byte *cipher, size_t cipherLen)
{
    using namespace CryptoPP;
    AutoSeededRandomPool prng;
    SecByteBlock iv(AES_BLOCKSIZE);
    prng.GenerateBlock(iv, iv.size());

    CBC_Mode<AES>::Encryption e;
    e.SetKeyWithIV(this->secret, this->secretLen, iv);

    // Add IV to the start of the cipher.
    std::copy(iv.begin(), iv.end(), cipher);

    ArraySource(plain, plainLen,
                true,
                new StreamTransformationFilter(
                    e,
                    new ArraySink(cipher + iv.size(), cipherLen - iv.size())));
};