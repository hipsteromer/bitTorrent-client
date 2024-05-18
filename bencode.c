#include "bencode.h"

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
        size_t length = 1;
        size_t i = 1;
        while (bencoded_string[i] != 'e') {
            size_t value_length = find_value_length(&bencoded_string[i]);
            if (value_length == 0) return 0;
            i += value_length;
            length += value_length;
        }
        return length + 1;
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
            decoded.val.str = (char *)malloc(length + 1);
            strncpy(decoded.val.str, start, length);
            decoded.val.str[length] = '\0';
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
        decoded.list_length = 0;
        size_t index = 1; // Skip 'l'
        while (bencoded_value[index] != 'e') {
            const char *element_start = &bencoded_value[index];
            DecodedValue element = decode_bencode(element_start);
            size_t element_length = find_value_length(element_start);
            // Append the decoded element to the list
            decoded.val.list = (DecodedValue *)realloc(decoded.val.list, (decoded.list_length + 1) * sizeof(DecodedValue));
            decoded.val.list[decoded.list_length++] = element;
            // Move index past the decoded element
            index += element_length;
        }
    } else {
        fprintf(stderr, "Invalid list format: %s\n", bencoded_value);
        exit(1);
    }
    return decoded;
}

// Decode a bencoded value (string, integer, or list)
DecodedValue decode_bencode(const char *bencoded_value) {
    if (is_digit(bencoded_value[0])) {
        return decode_string(bencoded_value);
    } else if (bencoded_value[0] == 'i') {
        return decode_integer(bencoded_value);
    } else if (bencoded_value[0] == 'l') {
        return decode_list(bencoded_value);
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
        for (size_t i = 0; i < decoded.list_length; i++) {
            free_decoded_value(decoded.val.list[i]);
        }
        free(decoded.val.list);
    }
}

// Print the decoded value
void print_decoded_value(DecodedValue decoded) {
    switch (decoded.type) {
        case DECODED_VALUE_TYPE_STR:
            printf("\"%s\"", decoded.val.str);
            break;
        case DECODED_VALUE_TYPE_INT:
            printf("%lld", decoded.val.integer);
            break;
        case DECODED_VALUE_TYPE_LIST:
            printf("[");
            for (size_t i = 0; i < decoded.list_length; i++) {
                print_decoded_value(decoded.val.list[i]);
                if (i < decoded.list_length - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        default:
            fprintf(stderr, "Unknown decoded value type\n");
            break;
    }
}
