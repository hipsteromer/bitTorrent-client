#include "info.h"
#include "bencode.h"
#include "sha1.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_torrent_file(const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", file_name);
        return NULL;
    }

    // Check for the file length
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    if (content == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(content, 1, file_size, file);
    if (read_size != file_size) {
        fprintf(stderr, "Error reading file\n");
        free(content);
        fclose(file);
        return NULL;
    }

    content[file_size] = '\0';
    fclose(file);
    return content;
}

int find_index(DecodedValue object, const char *str) {
    if (object.type != DECODED_VALUE_TYPE_DICT) {
        fprintf(stderr, "Decoded object is not a dictionary\n");
        exit(1);
    }

    for (size_t i = 0; i < object.size; i++) {
        if (strcmp(object.val.dict[i].key, str) == 0) {
            return i;
        }
    }

    return -1;
}

MetaInfo info_extract(const char *content) {
    MetaInfo file_contents = {};
    DecodedValue decoded_content = decode_bencode(content);
    if (decoded_content.type != DECODED_VALUE_TYPE_DICT) {
        fprintf(stderr, "Torrent file is not a valid bencode dictionary\n");
        exit(1);
    }

    // Check for index
    int announce_index = find_index(decoded_content, "announce");
    int info_index = find_index(decoded_content, "info");

    if (announce_index == -1) {
        fprintf(stderr, "Announce key not found\n");
        exit(1);
    }

    if (info_index == -1) {
        fprintf(stderr, "Info key not found\n");
        exit(1);
    }

    DecodedValue info_dict = decoded_content.val.dict[info_index].val;
    int pieces_index = find_index(info_dict, "pieces");
    int length_index = find_index(info_dict, "length");

    // Assign URL
    if (decoded_content.val.dict[announce_index].val.type == DECODED_VALUE_TYPE_STR) {
        const char *announce_value = decoded_content.val.dict[announce_index].val.val.str;
        file_contents.url = malloc(strlen(announce_value) + 1);
        if (file_contents.url != NULL) {
            strcpy(file_contents.url, announce_value);
        } else {
            fprintf(stderr, "Memory allocation for URL failed\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Announce value is not a string\n");
        exit(1);
    }

    // Assign length
    if (length_index != -1 && info_dict.val.dict[length_index].val.type == DECODED_VALUE_TYPE_INT) {
        file_contents.length = info_dict.val.dict[length_index].val.val.integer;
    } else {
        fprintf(stderr, "Length value not found or not an integer\n");
        exit(1);
    }

    // Assign piece length
    int piece_length_index = find_index(info_dict, "piece length");
    if (piece_length_index != -1 && info_dict.val.dict[piece_length_index].val.type == DECODED_VALUE_TYPE_INT) {
        file_contents.piece_length = info_dict.val.dict[piece_length_index].val.val.integer;
    } else {
        fprintf(stderr, "Piece length value not found or not an integer\n");
        exit(1);
    }

    // Assign info hash
    EncodedString bencoded_info = encode_decode(decoded_content.val.dict[info_index].val);
    unsigned char hash[SHA1_DIGEST_LENGTH];
    if (!sha1_hash((const unsigned char *)bencoded_info.str, bencoded_info.length, hash)) {
        fprintf(stderr, "SHA-1 hash computation failed\n");
        free(bencoded_info.str);
        exit(1);
    }

    // Allocate memory for the hash and copy it
    file_contents.info_hash = malloc(SHA1_DIGEST_LENGTH);
    if (file_contents.info_hash == NULL) {
        fprintf(stderr, "Memory allocation for info hash failed\n");
        free(bencoded_info.str);
        exit(1);
    }
    memcpy(file_contents.info_hash, hash, SHA1_DIGEST_LENGTH);

    // Assign pieces hashes
    if (pieces_index != -1 && info_dict.val.dict[pieces_index].val.type == DECODED_VALUE_TYPE_STR) {
        const char *pieces_value = info_dict.val.dict[pieces_index].val.val.str;
        size_t pieces_length = info_dict.val.dict[pieces_index].val.val.length;

        file_contents.num_pieces = pieces_length / SHA1_DIGEST_LENGTH;
        file_contents.pieces_hashes = malloc(file_contents.num_pieces * sizeof(unsigned char *));
        if (file_contents.pieces_hashes == NULL) {
            fprintf(stderr, "Memory allocation for pieces hashes array failed\n");
            free(bencoded_info.str);
            exit(1);
        }

        for (size_t i = 0; i < file_contents.num_pieces; i++) {
            file_contents.pieces_hashes[i] = malloc(SHA1_DIGEST_LENGTH);
            if (file_contents.pieces_hashes[i] == NULL) {
                fprintf(stderr, "Memory allocation for piece hash failed\n");
                for (size_t j = 0; j < i; j++) {
                    free(file_contents.pieces_hashes[j]);
                }
                free(file_contents.pieces_hashes);
                free(bencoded_info.str);
                exit(1);
            }
            memcpy(file_contents.pieces_hashes[i], pieces_value + i * 20, 20);
        }
    } else {
        fprintf(stderr, "Pieces value not found or not a string\n");
        free(bencoded_info.str);
        exit(1);
    }

    // Free the encoded string
    free(bencoded_info.str);

    return file_contents;
}

void free_info(MetaInfo info) {
    free(info.url);
    free(info.name);
    free(info.pieces);
    for (size_t i = 0; i < info.num_pieces; i++) {
        free(info.pieces_hashes[i]);
    }
    free(info.pieces_hashes);
    free(info.info_hash);
}

void print_meta_info(MetaInfo info) {
    if (info.url != NULL) {
        printf("Tracker URL: %s\n", info.url);
    }
    printf("Length: %zu\n", info.length);
    if (info.info_hash != NULL) {
        printf("Info Hash: ");
        for (int i = 0; i < SHA1_DIGEST_LENGTH; i++) {
            printf("%02x", (unsigned char)info.info_hash[i]);
        }
        printf("\n");
    }
    printf("Piece Length: %zu\n", info.piece_length);
    printf("Piece Hashes:\n");
    for (size_t i = 0; i < info.num_pieces; i++) {
        for (int j = 0; j < SHA1_DIGEST_LENGTH; j++) {
            printf("%02x", (unsigned char)info.pieces_hashes[i][j]);
        }
        printf("\n");
    }
}
