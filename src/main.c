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

        MetaInfo info = info_extract(torrent_file);

        if (sscanf(peer_info, "%255[^:]:%d", peer_ip, &peer_port) != 2) {
            fprintf(stderr, "Invalid peer address format. Expected <peer_ip>:<peer_port>\n");
            return 1;
        }

        int sockfd = create_socket();
        if (sockfd < 0) return 1;

        char *response = perform_peer_handshake(sockfd, info.info_hash, peer_ip, peer_port);

        if (response == NULL) {
            fprintf(stderr, "Failed to perform handshake with peer %s:%d\n", peer_ip, peer_port);
            close(sockfd);
            return 1;
        }

        printf("Peer ID: ");
        print_hex((unsigned char *)&response[48], 20);

        free(response);  // Free dynamically allocated memory
        free_info(info);
        close(sockfd);

    } else if (strcmp(command, "download_piece") == 0 && argc == 6) {
        const char *target_file = argv[3];
    const char *torrent_file = argv[4];
    int peer_index = atoi(argv[5]);

    // Get peers' IP
    char *content = read_torrent_file(torrent_file);
    if (content == NULL) {
        fprintf(stderr, "Failed to read torrent file: %s\n", torrent_file);
        return 1;
    }
    MetaInfo info = info_extract(content);
    PeersList peers_list = get_peers(info);

    // Handshake with the proper peer by index
    int sockfd = create_socket();
    if (sockfd < 0) return 1;

    char *response = perform_peer_handshake(sockfd, info.info_hash, peers_list.peers[peer_index].ip, peers_list.peers[peer_index].port);

    if (response == NULL) {
        fprintf(stderr, "Failed to perform handshake with peer %s:%d\n", peers_list.peers[peer_index].ip, peers_list.peers[peer_index].port);
        close(sockfd);
        free_peers(peers_list);
        free_info(info);
        free(content);
        return 1;
    }

    uint32_t piece_length = (peer_index == peers_list.count - 1) ? info.length - (peer_index * info.piece_length): info.piece_length;  // Length of the piece to download
    char *piece_data = download_piece(sockfd, peer_index, piece_length);
    if(!verify_piece(piece_data, piece_length, info.pieces_hashes[peer_index])){
        fprintf(stderr, "Failed to verify piece\n");
        free(piece_data);
        free_peers(peers_list);
        free_info(info);
        free(response);
        free(content);
        close(sockfd);
        return 1;
    }

    // Write the piece data to a file
    FILE *file = fopen(target_file, "wb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open target file: %s\n", target_file);
        free(piece_data);
        free_peers(peers_list);
        free_info(info);
        free(response);
        free(content);
        close(sockfd);
        return 1;
    }
    size_t bytes_written = fwrite(piece_data, 1, piece_length, file);
    if (bytes_written != piece_length) {
        fprintf(stderr, "Failed to write the complete piece data to file\n");
        fclose(file);
        free(piece_data);
        free_peers(peers_list);
        free_info(info);
        free(response);
        free(content);
        close(sockfd);
        return 1;
    }
    fclose(file);

    printf("Piece %d downloaded to %s", peer_index, target_file);

    // Cleanup
    free(piece_data);
    free_peers(peers_list);
    free_info(info);
    free(response);
    free(content);
    close(sockfd);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
