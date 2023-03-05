#ifndef MAIN_CRYPTO
#define MAIN_CRYPTO
#include <cryptopp/rsa.h>

namespace crypto
{
    class SymKey
    {
    protected:
        CryptoPP::byte *secret = nullptr;
        size_t secretLen = 0;

    public:
        /// @brief Generate a symmetric key with a random secret.
        void generateRandom();

        /// @brief Create a symmetric key.
        /// @param secret Secret key
        /// @param secretLen Length (in bytes) of the secret key
        SymKey(CryptoPP::byte *secret, size_t secretLen);
        SymKey(){};
        ~SymKey();

        size_t getSecretLen();
        void getSecret(CryptoPP::byte *secretBuffer, size_t secretBufferLen);

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
    };

    class SymKeyPasswordBased : public SymKey
    {
    private:
        CryptoPP::byte *salt = nullptr;
        size_t saltLen = 0;

        std::string password = "";
        void populateSecret();

    public:
        SymKeyPasswordBased(std::string password);
        SymKeyPasswordBased(std::string password, std::string saltSavePath);
        SymKeyPasswordBased(std::string password, CryptoPP::byte *salt, size_t saltLen);
        ~SymKeyPasswordBased();

        void saveSaltToFile(std::string saltSavePath);
    };

    enum KeyType
    {
        KeyTypePublic,
        KeyTypePrivate
    };

    class AsymKey
    {
    private:
        CryptoPP::RSA::PrivateKey *privateKey = nullptr;
        CryptoPP::RSA::PublicKey *publicKey = nullptr;
        size_t cipherLen = 0;

    public:
        ~AsymKey();

        /// @brief Generate and populate public and private RSA key.
        ///        If `RsaKey` is NOT empty, it will throw a `CryptoError`.
        /// @param size The size in bits of the RSA key to generate.
        void generate(unsigned int size = 2048);

        /// @brief Save private/public key to file.
        /// @param keyType Key type (private/public)
        /// @param path Path to file
        /// @param symKey Symmetric key used to encrypt the key
        void saveToFile(KeyType keyType, std::string path, SymKey *symKey = nullptr);
        void loadFromFile(KeyType keyType, std::string path, SymKey *symKey = nullptr);

        bool validate(KeyType keyType);
        size_t calculateCipherLen();

        void encrypt(CryptoPP::byte *plain, size_t plainLen, CryptoPP::byte *cipher, size_t cipherLen);
        void decrypt(CryptoPP::byte *cipher, size_t cipherLen,
                     CryptoPP::byte *plain, size_t plainLen,
                     size_t *outputLen = nullptr);
    };

    class CryptoError : public std::exception
    {
    protected:
        std::string message;

    public:
        CryptoError(const std::string &message = "A cryptographic error has occured.");

        const char *what() const noexcept override;
    };

    class DecryptionError : public CryptoError
    {
    public:
        DecryptionError(const std::string &message, bool prependCommonMessage = true);
    };
}

#endif /* MAIN_CRYPTO */
