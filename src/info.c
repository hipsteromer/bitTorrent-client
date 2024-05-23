#include "info.h"

char *read_torrent_file(const char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Faild to open");
        return NULL;
    }

    // Check for the file length
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    size_t read_size = fread(content, 1, file_size, file);
    if (read_size != file_size) {
        fprintf(stderr, "Error: reading file\n");
        free(content);
        fclose(file);
        return NULL;
    }

    content[file_size] = '\0';
    fclose(file);
    return content;
}

MetaInfo info_extract(char *content) {
    MetaInfo file_contents = {};
    DecodedValue decoded_content = decode_bencode(content);
    if(decoded_content.type != DECODED_VALUE_TYPE_DICT) {
        fprintf(stderr, "Torrent file is no a valid bencode dictionary");
        exit(1);
    }
    
}