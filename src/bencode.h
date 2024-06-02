#ifndef BENCODE_H
#define BENCODE_H

#include "decode.h"
#include "info.h"
#include <inttypes.h>


// A function that extracts the decoded value and returns it bencoded
char *encode_decode(DecodedValue decoded);

#endif // BENCODE_H