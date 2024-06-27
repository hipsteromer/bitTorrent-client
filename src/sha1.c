#include "sha1.h"

int sha1_hash(const unsigned char *data, size_t data_len, unsigned char *hash) {
    // Declare a pointer for the message digest context
    EVP_MD_CTX *mdctx;

    // Allocate and initialize a new message digest context
    if ((mdctx = EVP_MD_CTX_new()) == NULL) {
        return 0;
    }

    // Initialize the digest context to use the SHA-1 algorithm
    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    // Update the digest context with the data to be hashed
    if (1 != EVP_DigestUpdate(mdctx, data, data_len)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    // Finalize the digest operation and retrieve the hash
    if (1 != EVP_DigestFinal_ex(mdctx, hash, NULL)) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_free(mdctx);

    return 1;
}
