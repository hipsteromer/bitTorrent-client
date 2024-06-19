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

        // Read the torrent file and extract the info hash
        char *content = read_torrent_file(torrent_file);
        if (content == NULL) {
            fprintf(stderr, "Failed to read torrent file: %s\n", torrent_file);
            return 1;
        }
        MetaInfo info = info_extract(content);
        free(content);

        // Use info_hash from MetaInfo
        char info_hash[20];
        memcpy(info_hash, info.info_hash, 20);

        char handshake_packet[PACKET_LENGTH];
        construct_handshake_packet(handshake_packet, info_hash);

        int sockfd = create_socket();
        if (sockfd < 0) return 1;

        if (connect_to_peer(sockfd, peer_ip, peer_port) < 0) return 1;

        if (send_handshake(sockfd, handshake_packet) < 0) {
            close(sockfd);
            return 1;
        }

        char response[PACKET_LENGTH];
        int num_bytes = receive_response(sockfd, response, PACKET_LENGTH);
        if (num_bytes < 0) {
            close(sockfd);
            return 1;
        }

        printf("Peer ID: ");
        print_hex((unsigned char *)&response[48], 20);

        close(sockfd);
        free_info(info);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
