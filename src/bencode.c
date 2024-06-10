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

EncodedString bencode_string(const char *decoded, size_t len) {
    EncodedString result = {NULL, 0};

    if (decoded == NULL) return result;

    size_t encoded_len = snprintf(NULL, 0, "%zu", len) + len + 2;
    char *encoded_str = allocate_memory(encoded_len);

    if (encoded_str == NULL) return result;

    int prefix_len = snprintf(encoded_str, encoded_len, "%zu:", len);
    memcpy(encoded_str + prefix_len, decoded, len);

    result.str = encoded_str;
    result.length = encoded_len - 1;  // Exclude the null terminator in the length

    return result;
}

EncodedString bencode_integer(int64_t decoded) {
    EncodedString result = {NULL, 0};

    size_t encoded_len = snprintf(NULL, 0, "i%" PRId64 "e", decoded) + 1;
    char *encoded_str = allocate_memory(encoded_len);

    if (encoded_str == NULL) return result;

    snprintf(encoded_str, encoded_len, "i%" PRId64 "e", decoded);

    result.str = encoded_str;
    result.length = encoded_len - 1;  // Exclude the null terminator in the length

    return result;
}

EncodedString bencode_list(DecodedValue list) {
    EncodedString result = {NULL, 0};

    if (list.val.list == NULL || list.size == 0) return result;

    size_t value_size = 2; // initial size for 'l' and 'e'
    char *value = allocate_memory(value_size);

    if (value == NULL) return result;

    value[0] = 'l';
    value[1] = '\0';

    size_t total_length = 1; // length for 'l'
    for (size_t i = 0; i < list.size; i++) {
        EncodedString temp = encode_decode(list.val.list[i]);
        if (temp.str == NULL) {
            free(value);
            return result;
        }

        value_size += temp.length;
        value = realloc(value, value_size + 1);

        if (value == NULL) {
            fprintf(stderr, "Memory allocation failure\n");
            free(temp.str);
            return result;
        }

        memcpy(value + total_length, temp.str, temp.length);
        total_length += temp.length;
        free(temp.str);
    }

    value[total_length] = 'e';
    value[total_length + 1] = '\0';

    result.str = value;
    result.length = total_length + 1;  // include 'e'

    return result;
}

EncodedString bencode_dict(DecodedValue *dict) {
    EncodedString result = {NULL, 0};

    if (dict->val.dict == NULL || dict->size == 0) return result;

    // Sorting the dictionary before encoding it
    sort_dict(dict->val.dict, dict->size);

    size_t value_size = 2; // initial size for 'd' and 'e'
    char *value = allocate_memory(value_size);

    if (value == NULL) return result;

    value[0] = 'd';
    value[1] = '\0';

    size_t total_length = 1; // length for 'd'
    for (size_t i = 0; i < dict->size; i++) {
        EncodedString temp1 = bencode_string(dict->val.dict[i].key, strlen(dict->val.dict[i].key));
        EncodedString temp2 = encode_decode(dict->val.dict[i].val);

        if (temp1.str == NULL || temp2.str == NULL) {
            free(value);
            if (temp1.str) free(temp1.str);
            if (temp2.str) free(temp2.str);
            return result;
        }

        value_size += temp1.length + temp2.length;
        value = realloc(value, value_size + 1);

        if (value == NULL) {
            fprintf(stderr, "Memory allocation failure\n");
            free(temp1.str);
            free(temp2.str);
            return result;
        }

        memcpy(value + total_length, temp1.str, temp1.length);
        total_length += temp1.length;
        free(temp1.str);

        memcpy(value + total_length, temp2.str, temp2.length);
        total_length += temp2.length;
        free(temp2.str);
    }

    value[total_length] = 'e';
    value[total_length + 1] = '\0';

    result.str = value;
    result.length = total_length + 1;  // include 'e'

    return result;
}

EncodedString encode_decode(DecodedValue decoded) {
    switch (decoded.type) {
        case DECODED_VALUE_TYPE_STR:
            return bencode_string(decoded.val.str, decoded.val.length);
        case DECODED_VALUE_TYPE_INT:
            return bencode_integer(decoded.val.integer);
        case DECODED_VALUE_TYPE_LIST:
            return bencode_list(decoded);
        case DECODED_VALUE_TYPE_DICT:
            return bencode_dict(&decoded);
        default:
            fprintf(stderr, "Invalid type\n");
            EncodedString result = {NULL, 0};
            return result;
    }
}

void print_encoded_string(EncodedString encoded) {
    if (encoded.str == NULL || encoded.length == 0) {
        printf("Encoded string is empty or NULL.\n");
        return;
    }

    // Print each character as a hex value for clarity in binary data
    printf("Encoded String: ");
    for (size_t i = 0; i < encoded.length; ++i) {
        printf("%c", encoded.str[i]);
    }
    printf("\nLength: %zu\n", encoded.length);
}

