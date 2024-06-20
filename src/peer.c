#include "peer.h"
#include "info.h"

#include <arpa/inet.h>

void construct_handshake_packet(char *handshake_packet, const char *info_hash) {
    uint8_t protocolLength = 19;
    memset(handshake_packet, 0, PACKET_LENGTH);
    handshake_packet[0] = protocolLength;
    memcpy(&handshake_packet[1], PROTOCOL_STRING, protocolLength);
    memcpy(&handshake_packet[28], info_hash, 20);
    memcpy(&handshake_packet[48], PEER_ID, 20);
}

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }
    return sockfd;
}

int connect_to_peer(int sockfd, const char *peer_ip, int peer_port) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(peer_port);
    
    if (inet_pton(AF_INET, peer_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return -1;
    }
    return 0;
}

int send_handshake(int sockfd, const char *handshake_packet) {
    if (send(sockfd, handshake_packet, PACKET_LENGTH, 0) != PACKET_LENGTH) {
        perror("Send failed");
        return -1;
    }
    return 0;
}

int receive_response(int sockfd, char *response, size_t response_size) {
    ssize_t num_bytes = recv(sockfd, response, response_size, 0);
    if (num_bytes < 0) {
        perror("Receive failed");
        return -1;
    }
    return num_bytes;
}

char *perform_peer_handshake(int sockfd, char *torrent_file, char *peer_ip, int peer_port) {
    char *content = read_torrent_file(torrent_file);
    if (content == NULL) {
        fprintf(stderr, "Failed to read torrent file: %s\n", torrent_file);
        return NULL;  // Return NULL to indicate failure
    }

    MetaInfo info = info_extract(content);
    free(content);

    char info_hash[20];
    memcpy(info_hash, info.info_hash, 20);

    char handshake_packet[PACKET_LENGTH];
    construct_handshake_packet(handshake_packet, info_hash);

    if (connect_to_peer(sockfd, peer_ip, peer_port) < 0) {
        fprintf(stderr, "Failed to connect to peer: %s:%d\n", peer_ip, peer_port);
        free_info(info);
        return NULL;
    }

    if (send_handshake(sockfd, handshake_packet) < 0) {
        fprintf(stderr, "Failed to send handshake to peer: %s:%d\n", peer_ip, peer_port);
        close(sockfd);
        free_info(info);
        return NULL;
    }

    char *response = malloc(PACKET_LENGTH);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed for response\n");
        close(sockfd);
        free_info(info);
        return NULL;
    }

    int num_bytes = receive_response(sockfd, response, PACKET_LENGTH);
    if (num_bytes < 0) {
        fprintf(stderr, "Failed to receive response from peer: %s:%d\n", peer_ip, peer_port);
        close(sockfd);
        free(response);
        free_info(info);
        return NULL;
    }

    free_info(info);
    return response;
}

