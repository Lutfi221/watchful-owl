#ifndef MAIN_CRYPTO
#define MAIN_CRYPTO
#include <cryptopp/rsa.h>

namespace crypto
{
    class SymmetricKey
    {
    private:
        CryptoPP::byte *secret = nullptr;
        size_t secretLen = 0;
        CryptoPP::byte *salt = nullptr;
        size_t saltLen = 0;

        std::string password = "";
        void populateSecret();

    public:
        SymmetricKey(std::string password);
        SymmetricKey(std::string password, std::string saltSavePath);
        SymmetricKey(std::string password, CryptoPP::byte *salt, size_t saltLen);
        ~SymmetricKey();

        void generate(std::string password);

        /// @brief Calculate the needed cipher length from the plain length
        ///        while accounting for the IV and padding.
        /// @param plainLen Length (in bytes) of plain data.
        /// @return Length (in bytes) of cipher data.
        size_t calculateCipherLen(size_t plainLen);

        void encrypt(CryptoPP::byte *plain, size_t plainLen, CryptoPP::byte *cipher, size_t cipherLen);
        void encrypt(CryptoPP::ByteQueue *plain, CryptoPP::ByteQueue *cipher);

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
        void decrypt(CryptoPP::ByteQueue *cipher, CryptoPP::ByteQueue *plain);

        void saveSaltToFile(std::string saltSavePath);
    };

    enum KeyType
    {
        KeyTypePublic,
        KeyTypePrivate
    };

    class AsymmetricKey
    {
    private:
        CryptoPP::RSA::PrivateKey *privateKey = nullptr;
        CryptoPP::RSA::PublicKey *publicKey = nullptr;

    public:
        ~AsymmetricKey();

        /// @brief Generate and populate public and private RSA key.
        ///        If `RsaKey` is NOT empty, it will throw a `CryptoError`.
        /// @param size The size in bits of the RSA key to generate.
        void generate(unsigned int size = 2048);

        /// @brief Save private/public key to file.
        /// @param keyType Key type (private/public)
        /// @param path Path to file
        /// @param symKey Symmetric key used to encrypt the key
        void saveToFile(KeyType keyType, std::string path, SymmetricKey *symKey = nullptr);
        void loadFromFile(KeyType keyType, std::string path, SymmetricKey *symKey = nullptr);

        bool validate(KeyType keyType);
    };

    class CryptoError : public std::runtime_error
    {
        using CryptoError::runtime_error::runtime_error;
    };
}

#endif /* MAIN_CRYPTO */
