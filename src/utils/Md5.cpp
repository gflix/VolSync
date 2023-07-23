#include <unistd.h>
#include <openssl/evp.h>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utils/Md5.hpp>

namespace VolSync
{

ResponseGetChunkHash Md5::calculateChunkHash(int descriptor, uint64_t chunkSize)
{
    const EVP_MD* messageDigest = EVP_md5();
    EVP_MD_CTX* messageDigestContext = EVP_MD_CTX_new();
    uint8_t hashResult[EVP_MAX_MD_SIZE];
    unsigned int hashLength;

    if (!messageDigestContext)
    {
        throw std::runtime_error("error during initialization of the message digest");
    }
    if (!EVP_DigestInit_ex(messageDigestContext, messageDigest, nullptr))
    {
        EVP_MD_CTX_free(messageDigestContext);
        throw std::runtime_error("error during initialization of the message digest");
    }

    uint8_t buffer[chunkSize];
    auto bytesRead = read(descriptor, buffer, chunkSize);
    if (bytesRead != chunkSize)
    {
        EVP_MD_CTX_free(messageDigestContext);
        throw std::runtime_error(
            "unexpected number of bytes read from volume "
            "(" + std::to_string(bytesRead) + " != " + std::to_string(chunkSize) + ", " +
            std::string(strerror(errno)) + ")");
    }
    if (!EVP_DigestUpdate(messageDigestContext, buffer, chunkSize))
    {
        EVP_MD_CTX_free(messageDigestContext);
        throw std::runtime_error("error during update of the message digest");
    }
    if (!EVP_DigestFinal_ex(messageDigestContext, hashResult, &hashLength))
    {
        EVP_MD_CTX_free(messageDigestContext);
        throw std::runtime_error("error during calculation of the message digest");
    }

    EVP_MD_CTX_free(messageDigestContext);

    if (hashLength != ResponseGetChunkHash::hashLength)
    {
        throw std::runtime_error(
            "unexpected hash length "
            "(" + std::to_string(hashLength) + " != " + std::to_string(ResponseGetChunkHash::hashLength) + ")");
    }

    ResponseGetChunkHash result;
    memcpy(result.md5Hash, hashResult, ResponseGetChunkHash::hashLength);

    return result;
}

} /* namespace VolSync */
