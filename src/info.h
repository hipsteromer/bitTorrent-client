#ifndef INFO_H
#define INFO_H

#include "decode.h"
#include <stddef.h>

typedef struct MetaInfo {
    char *url;
    size_t length;
    char *name;
    size_t piece_length;
    char *pieces;
    unsigned char *info_hash;
    unsigned char **pieces_hashes; // Array of pointers to piece hashes
    size_t num_pieces;             // Number of pieces
} MetaInfo;

char *read_torrent_file(const char *file_name);
MetaInfo info_extract(const char *content);
void free_info(MetaInfo info);
void print_meta_info(MetaInfo info);

#endif // INFO_H
