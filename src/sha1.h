#ifndef SHA1_H
#define SHA1_H

#include <openssl/evp.h>

#define SHA1_DIGEST_LENGTH 20

// calculates sha1-hash for a char string. puts the hash itself in the 'hash' string
int sha1_hash(const unsigned char *data, size_t data_len, unsigned char *hash);

#endif /* SHA1_H */
