#include "info.h"
#include "decode.h"
#include "tracker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: bittorrent <command> <args>\n");
        return 1;
    }

    const char *command = argv[1];
    
    if (strcmp(command, "decode") == 0) {
        const char *encoded_str = argv[2];
        DecodedValue decoded = decode_bencode(encoded_str);
        print_decoded_value(decoded);
        printf("\n");
        free_decoded_value(decoded);
    } else if (strcmp(command, "info") == 0) {
        const char *file_name = argv[2];
        char *content = read_torrent_file(file_name);
        if (content == NULL) {
            fprintf(stderr, "Failed to read torrent file: %s\n", file_name);
            return 1;
        }
        MetaInfo info = info_extract(content);
        print_meta_info(info);
        free_info(info);
        free(content);
    } else if (strcmp(command, "peers") == 0) {
        const char *file_name = argv[2];
        char *content = read_torrent_file(file_name);
        if (content == NULL) {
            fprintf(stderr, "Failed to read torrent file: %s\n", file_name);
            return 1;
        }
        MetaInfo info = info_extract(content);
        PeersList peers = get_peers(info);
        print_peers(peers);
        free_peers(peers);
        free_info(info);
        free(content);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
