#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Enum for the type of data that DecodedValue can hold
typedef enum DecodedValueType {
    DECODED_VALUE_TYPE_STR,   // Represents a String
    DECODED_VALUE_TYPE_INT,   // Represents an Integer
    DECODED_VALUE_TYPE_LIST,  // Represents a List (array)
    DECODED_VALUE_TYPE_DICT,  // Represents a Dictionary
} DecodedValueType;

// Structure to hold either a string, integer, or list based on the specified type
typedef struct DecodedValue {
    DecodedValueType type;
    union {
        char *str;
        int64_t integer;
        struct {
            struct DecodedValue *list;
        };
        struct {
            struct KeyValPair *dict;
        };
    } val;
    size_t size;  // Number of elements in the list or dict
} DecodedValue;

// Structure to implement a type of key and value pair (for the dictionary)
typedef struct KeyValPair { 
    char *key;
    struct DecodedValue val;
} KeyValPair;

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

#endif // DECODE_H 
