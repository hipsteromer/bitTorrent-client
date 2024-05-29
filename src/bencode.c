#include "bencode.h"

char *bencode_string(char *decoded) {
    if (decoded == NULL) return NULL;
    printf("encoding string: ");
    size_t len = strlen(decoded);
    size_t encoded_len = snprintf(NULL, 0, "%zu", len) + len + 1;
    char *encoded_str = (char *)malloc(encoded_len);
    
    if (encoded_str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    sprintf(encoded_str, "%zu:%s", len, decoded);
    printf("%s\n", encoded_str);
    return encoded_str;
}

// Function to encode an integer
char *bencode_integer(int64_t decoded) {
    printf("encoding integer: ");

    size_t encoded_len = snprintf(NULL, 0, "%" PRId64, decoded) + 2;
    char *encoded_str = (char *)malloc(encoded_len);
    
    if (encoded_str == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    
    sprintf(encoded_str, "i%" PRId64 "e", decoded);
    printf("%s\n", encoded_str);
    return encoded_str;
}

// Function to encode a list
char *bencode_list(DecodedValue list) {
    if (list.val.list == NULL || list.size == 0) return NULL;
    printf("encoding list: ");

    char *value = (char *)malloc(2);
    size_t value_size = 2;
    
    for (size_t i = 0; i < list.size; i++) {
        char *temp = encode_decode(list.val.list[i]);
        value = realloc(value, strlen(temp) + value_size + 1);
        
        if (value == NULL) {
            fprintf(stderr, "Memory allocation failure\n");
            return NULL;
        }
        
        strcat(value, temp);
        value_size += strlen(temp);
        free(temp);
    }
    
    sprintf(value, "l%se", value);
    printf("%s", value);
    return value;
}

// Function to encode a dictionary
char *bencode_dict(DecodedValue dict) {
    if (dict.val.dict == NULL || dict.size == 0) return NULL;
    printf("encoding dict");


    char *value = (char *)malloc(2);
    size_t value_size = 2;
    
    for (size_t i = 0; i < dict.size; i++) {
        char *temp1 = bencode_string(dict.val.dict[i].key);
        char *temp2 = encode_decode(dict.val.dict[i].val);
        
        value = realloc(value, strlen(temp1) + strlen(temp2) + value_size + 1);
        
        if (value == NULL) {
            fprintf(stderr, "Memory allocation failure\n");
            return NULL;
        }
        
        strcat(value, temp1);
        strcat(value, temp2);
        value_size += strlen(temp1) + strlen(temp2);
        free(temp1);
        free(temp2);
    }
    
    sprintf(value, "d%se", value);
    printf("%s\n", value);
    return value;
}

// Function to encode a decoded value
char *encode_decode(DecodedValue decoded) {
    printf("trying ecodeing: \n");
    switch (decoded.type) {
        case DECODED_VALUE_TYPE_STR:
            return bencode_string(decoded.val.str);
        case DECODED_VALUE_TYPE_INT:
            return bencode_integer(decoded.val.integer);
        case DECODED_VALUE_TYPE_LIST:
            return bencode_list(decoded);
        case DECODED_VALUE_TYPE_DICT:
            return bencode_dict(decoded);
        default:
            fprintf(stderr, "Invalid type\n");
            return NULL;
    }
    printf("encoding ended!\n");
}