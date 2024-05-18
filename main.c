#include "bencode.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: bittorrent <command> <args>\n");
        return 1;
    }

    const char *command = argv[1];
    const char *encoded_str = argv[2];

    if (strcmp(command, "decode") == 0) {
        DecodedValue decoded = decode_bencode(encoded_str);
        print_decoded_value(decoded);
        printf("\n");
        free_decoded_value(decoded);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
