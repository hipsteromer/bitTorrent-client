#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// A type for the type of data that DecodedValue can hold
typedef enum DecodedValueType {
    DECODED_VALUE_TYPE_STR,  // Represents a string
    DECODED_VALUE_TYPE_INT,  // Represents an integer
} DecodedValueType;

// A structure to hold either a string of int based on the specified type
typedef struct DecodedValue {
    DecodedValueType type;
    union {
        char *str;
        int64_t integer;
    } val;
} DecodedValue;

bool is_digit(char c) {return c >= '0' && c <= '9';}

DecodedValue decode_bencode(const char *bencoded_value)
{
    DecodedValue decoded = {};
    size_t value_length = strlen(bencoded_value);

    if(is_digit(bencoded_value[0])){
        decoded.type = DECODED_VALUE_TYPE_STR;
        int length = atoi(bencoded_value);
        const char *colon_index = strchr(bencoded_value, ':');

        if (colon_index != NULL){
            const char *start = colon_index + 1;
            decoded.val.str = (char *)malloc(length + 1);
            strncpy(decoded.val.str, start, length);
            decoded.val.str[length] = '\0';
            return decoded;
        } else {
            fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
            exit(1);
        }

    } else if(bencoded_value[0] == 'i' &&
              bencoded_value[value_length - 1] == 'e'){
        decoded.type = DECODED_VALUE_TYPE_INT;
        char const *bencoded_int = &bencoded_value[1];   // pointer to traverse through the int
        size_t int_length = value_length - 2;            // length between i and e
        int64_t sign = 1;

        if (bencoded_int[0] == '-'){
            sign = -1;
            int_length--;
            bencoded_int++;
        }  

        for (size_t i = 0; i < int_length; i++){
            const char c = bencoded_int[i];

            if(!is_digit(c)){
                fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
                exit(1);
            }

            decoded.val.integer = (decoded.val.integer * 10) + c - '0';
        }
        decoded.val.integer *= sign;
        return decoded;      

    } else {
        fprintf(stderr, "Error: not supported\n");
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if(argc < 3) {
        fprintf(stderr, "Usage: bittorrent <command> <args>\n");
        return 1;
    }

    const char *command = argv[1];

    if(strcmp(command, "decode") == 0){
        const char *encoded_str = argv[2];

        DecodedValue decoded = decode_bencode(encoded_str);
        switch (decoded.type) {
        case DECODED_VALUE_TYPE_STR: {
            printf("\"%s\"\n", decoded.val.str);
            free(decoded.val.str);
        } break;
        case DECODED_VALUE_TYPE_INT: {
            printf("%ld\n", decoded.val.integer);
        } break;
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
