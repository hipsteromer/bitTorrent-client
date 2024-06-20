#include "info.h"
#include "decode.h"
#include "tracker.h"
#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_hex(const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

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
    } else if (strcmp(command, "handshake") == 0) {
    if (argc != 4) {
        fprintf(stderr, "Usage: bittorrent handshake <torrent file> <peer_ip>:<peer_port>\n");
        return 1;
    }

    const char *torrent_file = argv[2];
    const char *peer_info = argv[3];
    char peer_ip[256];
    int peer_port;

    if (sscanf(peer_info, "%255[^:]:%d", peer_ip, &peer_port) != 2) {
        fprintf(stderr, "Invalid peer address format. Expected <peer_ip>:<peer_port>\n");
        return 1;
    }

    int sockfd = create_socket();
    if (sockfd < 0) return 1;

    char *response = perform_peer_handshake(sockfd, torrent_file, peer_ip, peer_port);

    if (response == NULL) {
        fprintf(stderr, "Failed to perform handshake with peer %s:%d\n", peer_ip, peer_port);
        close(sockfd);
        return 1;
    }


    printf("Peer ID: ");
    print_hex((unsigned char *)&response[48], 20);

    free(response);  // Free dynamically allocated memory
    close(sockfd);

    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}

