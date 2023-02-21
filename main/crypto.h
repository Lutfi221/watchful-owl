#ifndef MAIN_CRYPTO
#define MAIN_CRYPTO
#include <cryptopp/rsa.h>

namespace crypto
{
    enum KeyType
    {
        KeyTypePublic,
        KeyTypePrivate
    };

    class RsaKey
    {
    private:
        CryptoPP::RSA::PrivateKey *privateKey = nullptr;
        CryptoPP::RSA::PublicKey *publicKey = nullptr;

    public:
        ~RsaKey();

        /// @brief Generate and populate public and private RSA key.
        ///        If `RsaKey` is NOT empty, it will throw a `CryptoError`.
        /// @param size The size in bits of the RSA key to generate.
        void generate(unsigned int size = 2048);

        /// @brief Save private/public key to file.
        /// @param keyType Key type (private/public)
        /// @param path Path to file
        /// @param password Password used to encrypt the saved key
        void saveToFile(KeyType keyType, std::string path, std::string password = "");

        bool validate(KeyType keyType);
    };

    class SymmetricKey
    {
    private:
        CryptoPP::byte *secret = nullptr;
        size_t secretLen;

    public:
        SymmetricKey(std::string password, CryptoPP::byte *salt, size_t saltLen);
        ~SymmetricKey();

        /// @brief Calculate the needed cipher length from the plain length
        ///        while accounting for the IV and padding.
        /// @param plainLen Length (in bytes) of plain data.
        /// @return Length (in bytes) of cipher data.
        size_t calculateCipherLen(size_t plainLen);

        void encrypt(CryptoPP::byte *plain, size_t plainLen, CryptoPP::byte *cipher, size_t cipherLen);

        /// @brief Decrypt cipher
        /// @param cipher
        /// @param cipherLen
        /// @param plainBuffer Where the decrypted plain text will be put.
        /// @param plainBufferLen
        /// @param outputLen Where the actual decrypted output length (in bytes) will be put.
        void decrypt(
            CryptoPP::byte *cipher, size_t cipherLen,
            CryptoPP::byte *plainBuffer, size_t plainBufferLen,
            size_t *outputLen = nullptr);
    };

    class CryptoError : public std::runtime_error
    {
        using CryptoError::runtime_error::runtime_error;
    };
}

#endif /* MAIN_CRYPTO */
