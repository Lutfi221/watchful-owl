#include <filesystem>
#include <memory>
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
#define SALT_LEN 32
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

void crypto::RsaKey::saveToFile(crypto::KeyType keyType, std::string path, SymmetricKey *symKey)
{
    using namespace CryptoPP;
    FileSink file(path.c_str());
    ByteQueue q;
    auto *key = keyType == KeyTypePrivate ? this->privateKey : this->publicKey;

    if (key == nullptr)
        throw CryptoError("Cannot save a private/public key that is not initialized");

    INFO("Saving {}", keyType == KeyTypePrivate ? "private key" : "public key");
    DEBUG("Copy key to byte queue");
    key->Save(q);

    if (symKey != nullptr)
    {
        ByteQueue cipher;
        symKey->encrypt(&q, &cipher);
        q.Clear();
        cipher.CopyTo(q);
    };

    DEBUG("Copy from byte queue -> base 64 encoder -> file at `{}`", path);
    Base64Encoder encoder(new Redirector(file));
    q.CopyTo(encoder);
    encoder.MessageEnd();
}

void crypto::RsaKey::loadFromFile(crypto::KeyType keyType, std::string path, crypto::SymmetricKey *symKey)
{
    using namespace CryptoPP;
    std::unique_ptr<ByteQueue> q(new ByteQueue);
    Base64Decoder decoder(new Redirector(*(q.get())));

    DEBUG("Read file `{}` -> decode base 64 -> byte queue", path);
    FileSource fs(path.c_str(), true, new Redirector(decoder));
    decoder.MessageEnd();

    if (symKey != nullptr)
    {
        std::unique_ptr<ByteQueue> plain(new ByteQueue);
        DEBUG("Decrypt key");
        symKey->decrypt(q.get(), plain.get());
        q.swap(plain);
    }

    if (keyType == KeyTypePrivate)
    {
        assert(this->privateKey == nullptr);
        this->privateKey = new RSA::PrivateKey;
        this->privateKey->Load(*(q.get()));
        return;
    };

    assert(this->publicKey == nullptr);
    this->publicKey = new RSA::PublicKey;
    this->publicKey->Load(*(q.get()));
    return;
}

bool crypto::RsaKey::validate(crypto::KeyType keyType)
{
    CryptoPP::AutoSeededRandomPool prng;
    if (keyType == KeyTypePrivate)
    {
        if (this->privateKey == nullptr)
            throw CryptoError("Cannot validate private key as it's not loaded or generated yet.");
        return this->privateKey->Validate(prng, 2);
    };

    if (this->publicKey == nullptr)
        throw CryptoError("Cannot validate public key as it's not loaded or generated yet.");
    return this->publicKey->Validate(prng, 2);
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

crypto::SymmetricKey::SymmetricKey(std::string password)
{
    using namespace CryptoPP;
    DEBUG("Generate random salt");
    AutoSeededRandomPool prng;

    this->salt = new byte[SALT_LEN];
    this->saltLen = SALT_LEN;
    prng.GenerateBlock(salt, SALT_LEN);

    this->populateSecret();
}
crypto::SymmetricKey::SymmetricKey(std::string password, std::string saltSavePath)
{
    using namespace CryptoPP;
    this->password = password;

    DEBUG("Read salt file `{}`", saltSavePath);
    ByteQueue q;
    Base64Decoder decoder(new Redirector(q));
    FileSource fs(saltSavePath.c_str(), true, new Redirector(decoder));
    decoder.MessageEnd();

    this->saltLen = q.CurrentSize();
    DEBUG("Salt length: {} bytes", this->saltLen);

    this->salt = new byte[this->saltLen];
    q.Get(this->salt, this->saltLen);
    this->populateSecret();
};

crypto::SymmetricKey::SymmetricKey(std::string password, CryptoPP::byte *salt, size_t saltLen)
    : secretLen(AES_KEY_LEN)
{
    this->password = password;
    this->salt = new CryptoPP::byte[saltLen];
    this->saltLen = saltLen;
    std::copy(salt, salt + saltLen, this->salt);

    this->populateSecret();
}

crypto::SymmetricKey::~SymmetricKey()
{
    delete[] this->secret;
    delete[] this->salt;
}

void crypto::SymmetricKey::populateSecret()
{
    if (this->secret != nullptr)
        throw CryptoError("Secret already initialized");

    this->secret = new CryptoPP::byte[AES_KEY_LEN];
    this->secretLen = AES_KEY_LEN;

    std::unique_ptr<CryptoPP::byte> p(new CryptoPP::byte[password.size()]);
    std::copy(password.begin(), password.end(), p.get());

    derivePassword(this->secret, this->secretLen,
                   p.get(), password.size(),
                   this->salt, this->saltLen);
};

void crypto::SymmetricKey::encrypt(CryptoPP::ByteQueue *plain, CryptoPP::ByteQueue *cipher)
{
    using namespace CryptoPP;
    auto inLen = plain->CurrentSize();

    // TODO: Use secure bytes from CryptoPP
    DEBUG("Copy message to a byte array for encryption");
    byte *in = new byte[inLen];
    plain->Get(in, inLen);

    auto outLen = this->calculateCipherLen(inLen);
    DEBUG("plainLen: `{} bytes`, cipherLen: `{} bytes`", inLen, outLen);
    byte *out = new byte[outLen];

    DEBUG("Encrypt message");
    this->encrypt(in, inLen, out, outLen);

    DEBUG("Put encrypted message into cipher byte queue");
    cipher->Put(out, outLen);

    delete[] in;
    delete[] out;
};
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

void crypto::SymmetricKey::decrypt(CryptoPP::ByteQueue *cipher, CryptoPP::ByteQueue *plain)
{
    using namespace CryptoPP;
    auto inLen = cipher->CurrentSize();

    DEBUG("Copy message to a byte array for decryption");
    byte *in = new byte[inLen];
    cipher->Get(in, inLen);

    size_t actualOutLen;
    auto outBufferLen = inLen;
    byte *outBuffer = new byte[outBufferLen];

    DEBUG("Decrypt message");
    this->decrypt(in, inLen, outBuffer, outBufferLen, &actualOutLen);

    DEBUG("Put decrypted message into plain byte queue");
    plain->Put(outBuffer, actualOutLen);

    delete[] in;
    delete[] outBuffer;
};
void crypto::SymmetricKey::decrypt(
    CryptoPP::byte *cipher, size_t cipherLen,
    CryptoPP::byte *plainBuffer, size_t plainBufferLen,
    size_t *outputLen)
{
    using namespace CryptoPP;

    byte iv[AES_BLOCKSIZE];
    std::copy(cipher, cipher + AES_BLOCKSIZE, iv);

    CBC_Mode<AES>::Decryption d;
    d.SetKeyWithIV(this->secret, this->secretLen, iv);

    MeterFilter meter(new ArraySink(plainBuffer, plainBufferLen));

    ArraySource ar(cipher + AES_BLOCKSIZE,
                   cipherLen - AES_BLOCKSIZE,
                   true,
                   new StreamTransformationFilter(
                       d,
                       new Redirector(meter)));

    if (outputLen != nullptr)
    {
        *outputLen = meter.GetTotalBytes();
    }
};

size_t crypto::SymmetricKey::calculateCipherLen(size_t plainLen)
{
    return CryptoPP::RoundUpToMultipleOf(plainLen + AES_BLOCKSIZE, AES_BLOCKSIZE);
}

void crypto::SymmetricKey::saveSaltToFile(std::string saltSavePath)
{
    using namespace CryptoPP;
    Base64Encoder encoder(new FileSink(saltSavePath.c_str()));

    DEBUG("Save salt to `{}`", saltSavePath);
    encoder.Put(this->salt, this->saltLen);
    encoder.MessageEnd();
}