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
    };

    class CryptoError : public std::runtime_error
    {
        using CryptoError::runtime_error::runtime_error;
    };
}

#endif /* MAIN_CRYPTO */
