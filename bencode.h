#ifndef BENCODE_H
#define BENCODE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Enum for the type of data that DecodedValue can hold
typedef enum DecodedValueType {
    DECODED_VALUE_TYPE_STR,   // Represents a string
    DECODED_VALUE_TYPE_INT,   // Represents an integer
    DECODED_VALUE_TYPE_LIST,  // Represents a list (array)
} DecodedValueType;

// Structure to hold either a string, integer, or list based on the specified type
typedef struct DecodedValue {
    DecodedValueType type;
    union {
        char *str;
        int64_t integer;
        struct DecodedValue *list;
    } val;
    size_t list_length;  // Number of elements in the list (if type is DECODED_VALUE_TYPE_LIST)
} DecodedValue;

// Check if a character is a digit
bool is_digit(char c);

// Find the length of the bencoded value
size_t find_value_length(const char *bencoded_string);

// Decode a bencoded value
DecodedValue decode_bencode(const char *bencoded_value);

// Free the memory allocated for a decoded value
void free_decoded_value(DecodedValue decoded);

// Print the decoded value
void print_decoded_value(DecodedValue decoded);

#endif /* BENCODE_H */
