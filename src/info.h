#ifndef INFO_H
#define INFO_H

#include "bencode.h"

typedef struct MetaInfo {
    char *url;
    size_t length;
    char *name;
    size_t piece_length;
    char * pieces;
} MetaInfo;

// A function that reads and puts file contents in a string
char *read_torrent_file(const char *file_name);

#endif // INFO_H