#include "bencode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

// Helper function for memory allocation with error checking
char *allocate_memory(size_t size) {
    char *ptr = (char *)malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
    }
    return ptr;
}

// Comparison function to be used with qsort
int compare_keyval_pairs(const void *a, const void *b) {
    KeyValPair *pairA = (KeyValPair *)a;
    KeyValPair *pairB = (KeyValPair *)b;
    return strcmp(pairA->key, pairB->key);
}

// Function to sort a dictionary by its keys
void sort_dict(KeyValPair *dict, size_t size) {
    qsort(dict, size, sizeof(KeyValPair), compare_keyval_pairs);
}


char *bencode_string(const char *decoded) {
    if (decoded == NULL) return NULL;
    
    size_t len = strlen(decoded);
    size_t encoded_len = snprintf(NULL, 0, "%zu", len) + len + 2;
    char *encoded_str = allocate_memory(encoded_len);
    
    if (encoded_str == NULL) return NULL;

    snprintf(encoded_str, encoded_len, "%zu:%s", len, decoded);
    return encoded_str;
}

char *bencode_integer(int64_t decoded) {
    size_t encoded_len = snprintf(NULL, 0, "i%" PRId64 "e", decoded) + 1;
    char *encoded_str = allocate_memory(encoded_len);

    if (encoded_str == NULL) return NULL;

    snprintf(encoded_str, encoded_len, "i%" PRId64 "e", decoded);
    return encoded_str;
}

char *bencode_list(DecodedValue list) {
    if (list.val.list == NULL || list.size == 0) return NULL;
    
    size_t value_size = 2; // initial size for 'l' and 'e'
    char *value = allocate_memory(value_size);
    
    if (value == NULL) return NULL;

    strcpy(value, "l");

    for (size_t i = 0; i < list.size; i++) {
        char *temp = encode_decode(list.val.list[i]);
        if (temp == NULL) {
            free(value);
            return NULL;
        }

        value_size += strlen(temp);
        value = realloc(value, value_size + 1);
        
        if (value == NULL) {
            fprintf(stderr, "Memory allocation failure\n");
            free(temp);
            return NULL;
        }

        strcat(value, temp);
        free(temp);
    }
    
    strcat(value, "e");
    return value;
}

char *bencode_dict(DecodedValue *dict) {
    if (dict->val.dict == NULL || dict->size == 0) return NULL;
    
    // Sorting the dictionary before encoding it
    sort_dict(dict->val.dict, dict->size);

    size_t value_size = 2; // initial size for 'd' and 'e'
    char *value = allocate_memory(value_size);
    
    if (value == NULL) return NULL;

    strcpy(value, "d");

    for (size_t i = 0; i < dict->size; i++) {
        char *temp1 = bencode_string(dict->val.dict[i].key);
        char *temp2 = encode_decode(dict->val.dict[i].val);
        
        if (temp1 == NULL || temp2 == NULL) {
            free(value);
            if (temp1) free(temp1);
            if (temp2) free(temp2);
            return NULL;
        }

        value_size += strlen(temp1) + strlen(temp2);
        value = realloc(value, value_size + 1);

        if (value == NULL) {
            fprintf(stderr, "Memory allocation failure\n");
            free(temp1);
            free(temp2);
            return NULL;
        }

        strcat(value, temp1);
        strcat(value, temp2);
        free(temp1);
        free(temp2);
    }
    
    strcat(value, "e");
    return value;
}

char *encode_decode(DecodedValue decoded) {
    switch (decoded.type) {
        case DECODED_VALUE_TYPE_STR:
            return bencode_string(decoded.val.str);
        case DECODED_VALUE_TYPE_INT:
            return bencode_integer(decoded.val.integer);
        case DECODED_VALUE_TYPE_LIST:
            return bencode_list(decoded);
        case DECODED_VALUE_TYPE_DICT:
            return bencode_dict(&decoded);
        default:
            fprintf(stderr, "Invalid type\n");
            return NULL;
    }
}
