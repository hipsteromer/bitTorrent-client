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

// find the index(location) inside a decoded value by string
int find_index(DecodedValue object, const char *str);

// constucts a string of the Meta info (useful for ncurses)
char* meta_info_to_string(MetaInfo info);

// reads the contents of a file and making it a string (allocated on the heap)
char *read_torrent_file(const char *file_name);

// takes away and orgenaize the meta info inside a (torrent) file
MetaInfo info_extract(const char *content);

// free the memory of the info allocated on the heap
void free_info(MetaInfo info);

// prints the meta info (useless with ncurses)
void print_meta_info(MetaInfo info);

#endif // INFO_H
