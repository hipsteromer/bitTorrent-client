#include "decode.h"

// Check if a character is a digit
bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

// Find the length of the bencoded value
size_t find_value_length(const char *bencoded_string) {
    if (bencoded_string[0] == 'i') {
        // Integer value
        char *ptr = strchr(bencoded_string, 'e');
        if (ptr != NULL) {
            return (ptr - bencoded_string) + 1;
        }
    } else if (is_digit(bencoded_string[0])) {
        // String value
        const char *colon = strchr(bencoded_string, ':');
        if (colon != NULL) {
            int length = atoi(bencoded_string);
            return (colon - bencoded_string + 1) + length;
        }
    } else if (bencoded_string[0] == 'l' || bencoded_string[0] == 'd') {
        // List or dictionary value
        size_t i = 1;
        while (bencoded_string[i] != 'e') {
            size_t value_length = find_value_length(&bencoded_string[i]);
            if (value_length == 0) return 0;
            i += value_length;
        }
        return i + 1;
    }
    return 0;
}

// Decode a bencoded string
DecodedValue decode_string(const char *bencoded_value) {
    DecodedValue decoded = {};
    if (is_digit(bencoded_value[0])) {
        decoded.type = DECODED_VALUE_TYPE_STR;
        int length = atoi(bencoded_value);
        const char *colon_index = strchr(bencoded_value, ':');
        if (colon_index != NULL) {
            const char *start = colon_index + 1;
            decoded.val.str = (char *)malloc(length);
            if (decoded.val.str == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            memcpy(decoded.val.str, start, length); // Use memcpy for binary data
            decoded.val.length = length;
        } else {
            fprintf(stderr, "Invalid encoded value: %s\n", bencoded_value);
            exit(1);
        }
    } else {
        fprintf(stderr, "Invalid string format: %s\n", bencoded_value);
        exit(1);
    }
    return decoded;
}

// Decode a bencoded integer
DecodedValue decode_integer(const char *bencoded_value) {
    DecodedValue decoded = {};
    size_t value_length = find_value_length(bencoded_value);
    if (value_length != 0) {
        decoded.type = DECODED_VALUE_TYPE_INT;
        const char *int_start = bencoded_value + 1; // Skip 'i'
        size_t int_length = value_length - 2; // Exclude 'i' and 'e'
        char *int_str = strndup(int_start, int_length);
        if (int_str == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        decoded.val.integer = atoll(int_str);
        free(int_str);
    } else {
        fprintf(stderr, "Invalid integer format: %s\n", bencoded_value);
        exit(1);
    }
    return decoded;
}

// Decode a bencoded list
DecodedValue decode_list(const char *bencoded_value) {
    DecodedValue decoded = {};
    if (bencoded_value[0] == 'l') {
        decoded.type = DECODED_VALUE_TYPE_LIST;
        decoded.val.list = NULL;
        decoded.size = 0;
        size_t index = 1; // Skip 'l'
        while (bencoded_value[index] != 'e') {
            const char *element_start = &bencoded_value[index];
            DecodedValue element = decode_bencode(element_start);
            size_t element_length = find_value_length(element_start);

            // Append the decoded element to the list
            decoded.val.list = (DecodedValue *)realloc(decoded.val.list, (decoded.size + 1) * sizeof(DecodedValue));
            if (decoded.val.list == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                exit(1);
            }
            decoded.val.list[decoded.size++] = element;

            // Move index past the decoded element
            index += element_length;
        }
    } else {
        fprintf(stderr, "Invalid list format: %s\n", bencoded_value);
        exit(1);
    }
    return decoded;
}

// Decode a bencoded dictionary
DecodedValue decode_dict(const char *bencoded_value) {
    DecodedValue decoded = {};
    if (bencoded_value[0] == 'd') {
        decoded.type = DECODED_VALUE_TYPE_DICT;
        decoded.val.dict = NULL;
        decoded.size = 0;
        size_t index = 1; // Skip 'd'
        
        while (bencoded_value[index] != 'e') {
            // Decode the key
            DecodedValue key_obj = decode_bencode(&bencoded_value[index]);
            if (key_obj.type != DECODED_VALUE_TYPE_STR) {
                fprintf(stderr, "Dictionary key is not a string: %s\n", bencoded_value);
                exit(1);
            }
            index += find_value_length(&bencoded_value[index]);

            // Decode the value
            DecodedValue value = decode_bencode(&bencoded_value[index]);
            size_t value_length = find_value_length(&bencoded_value[index]);
            index += value_length;

            // Append the key-value pair to the dictionary
            decoded.val.dict = (KeyValPair *)realloc(decoded.val.dict, (decoded.size + 1) * sizeof(KeyValPair));
            if (decoded.val.dict == NULL) {
                fprintf(stderr, "Memory reallocation failed\n");
                exit(1);
            }
            decoded.val.dict[decoded.size].key = strndup(key_obj.val.str, key_obj.val.length);
            if (decoded.val.dict[decoded.size].key == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            decoded.val.dict[decoded.size].val = value;
            decoded.size++;
        }
    } else {
        fprintf(stderr, "Invalid dictionary format: %s\n", bencoded_value);
        exit(1);
    }
    return decoded;
}

// Decode a bencoded value (string, integer, list, or dictionary)
DecodedValue decode_bencode(const char *bencoded_value) {
    if (is_digit(bencoded_value[0])) {
        return decode_string(bencoded_value);
    } else if (bencoded_value[0] == 'i') {
        return decode_integer(bencoded_value);
    } else if (bencoded_value[0] == 'l') {
        return decode_list(bencoded_value);
    } else if (bencoded_value[0] == 'd') {
        return decode_dict(bencoded_value);
    } else {
        fprintf(stderr, "Unsupported encoded value: %s\n", bencoded_value);
        exit(1);
    }
}

// Free the memory allocated for a decoded value
void free_decoded_value(DecodedValue decoded) {
    if (decoded.type == DECODED_VALUE_TYPE_STR) {
        free(decoded.val.str);
    } else if (decoded.type == DECODED_VALUE_TYPE_LIST) {
        for (size_t i = 0; i < decoded.size; i++) {
            free_decoded_value(decoded.val.list[i]);
        }
        free(decoded.val.list);
    } else if (decoded.type == DECODED_VALUE_TYPE_DICT) {
        for (size_t i = 0; i < decoded.size; i++) {
            free(decoded.val.dict[i].key);
            free_decoded_value(decoded.val.dict[i].val);
        }
        free(decoded.val.dict);
    }
}

// Print the decoded value in a safe manner
void print_decoded_value(DecodedValue decoded) {
    switch (decoded.type) {
        case DECODED_VALUE_TYPE_STR:
            printf("\"");
            for (size_t i = 0; i < decoded.val.length; i++) {
                printf("%c", decoded.val.str[i]);
            }
            printf("\"");
            break;
        case DECODED_VALUE_TYPE_INT:
            printf("%lld", decoded.val.integer);
            break;
        case DECODED_VALUE_TYPE_LIST:
            printf("[");
            for (size_t i = 0; i < decoded.size; i++) {
                print_decoded_value(decoded.val.list[i]);
                if (i < decoded.size - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case DECODED_VALUE_TYPE_DICT:
            printf("{");
            for (size_t i = 0; i < decoded.size; i++) {
                printf("\"%s\": ", decoded.val.dict[i].key);
                print_decoded_value(decoded.val.dict[i].val);
                if (i < decoded.size - 1) {
                    printf(", ");
                }
            }
            printf("}");
            break;
        default:
            fprintf(stderr, "Unknown decoded value type\n");
            break;
    }
}