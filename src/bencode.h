#ifndef BENCODE_H
#define BENCODE_H

#include "decode.h"
#include "info.h"
#include <inttypes.h>

// Struct for encoded string and its length
typedef struct {
    char *str;
    size_t length;
} EncodedString;

// A function that extracts the decoded value and returns it bencoded
EncodedString encode_decode(DecodedValue decoded);

// Function to print an EncodedString
void print_encoded_string(EncodedString encoded);

#endif // BENCODE_H
