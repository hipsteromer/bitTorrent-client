#ifndef INFO_H
#define INFO_H

#include "bencode.h"

// Struct that holds the info essentials
typedef struct MetaInfo {
    char *url;
    size_t length;
    char *name;
    size_t piece_length;
    char *pieces;
} MetaInfo;

// A function that reads and puts file contents in a string
char *read_torrent_file(const char *file_name);

// A function that finds the index of the first appearance of a key in a dictionary
int find_index(DecodedValue object, const char *str);

// A function that extracts the info of the file and assigns it to a MetaInfo struct
MetaInfo info_extract(const char *content);

// A function that frees allocated memory
void free_info(MetaInfo info);

// A function that prints the contents of the struct MetaInfo
void print_meta_info(MetaInfo info);

#endif // INFO_H
