#include "peer.h"
#include "info.h"
#include "sha1.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void construct_handshake_packet(char *handshake_packet, const char *info_hash) {
    uint8_t protocolLength = 19;
    memset(handshake_packet, 0, PACKET_LENGTH);
    handshake_packet[0] = protocolLength;
    memcpy(&handshake_packet[1], PROTOCOL_STRING, protocolLength);
    memcpy(&handshake_packet[28], info_hash, 20);
    memcpy(&handshake_packet[48], PEER_ID, 20);
}

void construct_request_message(char *request_packet, uint32_t index, uint32_t begin, uint32_t length) {
    uint32_t length_prefix = htonl(13);  // 4 (index) + 4 (begin) + 4 (length) + 1 (message id)
    uint8_t message_id = 6;  // Request message ID
    uint32_t net_index = htonl(index);
    uint32_t net_begin = htonl(begin);
    uint32_t net_length = htonl(length);

    // Copy data to request_packet
    memcpy(request_packet, &length_prefix, 4);  // Length prefix
    memcpy(request_packet + 4, &message_id, 1);  // Message ID
    memcpy(request_packet + 5, &net_index, 4);  // Piece index
    memcpy(request_packet + 9, &net_begin, 4);  // Begin offset
    memcpy(request_packet + 13, &net_length, 4);  // Requested length
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

char *perform_peer_handshake(int sockfd, const unsigned char *info_hash, const char *peer_ip, int peer_port){
    char handshake_packet[PACKET_LENGTH];
    construct_handshake_packet(handshake_packet, info_hash);

    if (connect_to_peer(sockfd, peer_ip, peer_port) < 0) {
        fprintf(stderr, "Failed to connect to peer: %s:%d\n", peer_ip, peer_port);
        return NULL;
    }

    if (send_handshake(sockfd, handshake_packet) < 0) {
        fprintf(stderr, "Failed to send handshake to peer: %s:%d\n", peer_ip, peer_port);
        close(sockfd);
        return NULL;
    }

    char *response = malloc(PACKET_LENGTH);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed for response\n");
        close(sockfd);
        return NULL;
    }

    int num_bytes = receive_response(sockfd, response, PACKET_LENGTH);
    if (num_bytes < 0) {
        fprintf(stderr, "Failed to receive response from peer: %s:%d\n", peer_ip, peer_port);
        close(sockfd);
        free(response);
        return NULL;
    }

    return response;
}

int read_packet(int sockfd, char **packet) {
    uint32_t length_prefix;
    
    // Step 1: Receive the first 4 bytes (length prefix)
    ssize_t bytes_received = recv(sockfd, &length_prefix, LENGTH_PREFIX_SIZE, 0);
    if (bytes_received != LENGTH_PREFIX_SIZE) {
        perror("Failed to read length prefix");
        return -1;
    }

    // Convert length prefix from network byte order to host byte order
    length_prefix = ntohl(length_prefix);

    // Step 2: Allocate memory for the packet
    *packet = (char *)malloc(length_prefix);
    if (*packet == NULL) {
        perror("Memory allocation failed");
        return -1;
    }

    // Step 3: Receive the rest of the packet
    bytes_received = recv(sockfd, *packet, length_prefix, 0);
    if (bytes_received != length_prefix) {
        perror("Failed to read the full packet");
        free(*packet);
        return -1;
    }

    return length_prefix; // Return the length of the packet
}

int read_piece_message(int sockfd, uint32_t *index, uint32_t *begin, char **block, uint32_t *block_length){
    uint32_t length_prefix;
    ssize_t bytes_received = recv(sockfd, &length_prefix, 4, 0);
    if (bytes_received != 4) {
        perror("Failed to read length prefix");
        return -1;
    }
    length_prefix = ntohl(length_prefix);

    // Verify message ID
    uint8_t message_id;
    bytes_received = recv(sockfd, &message_id, 1, 0);
    if (bytes_received != 1 || message_id != PIECE) {
        perror("Invalid or unexpected message ID");
        return -1;
    }

    // Read the index and begin fields
    bytes_received = recv(sockfd, index, 4, 0);
    if (bytes_received != 4) {
        perror("Failed to read piece index");
        return -1;
    }
    *index = ntohl(*index);

    bytes_received = recv(sockfd, begin, 4, 0);
    if (bytes_received != 4) {
        perror("Failed to read piece begin offset");
        return -1;
    }
    *begin = ntohl(*begin);

    // Calculate block length
    *block_length = length_prefix - 9; // message ID (1 byte) + begin and index (8 bytes)

    printf("block length calculated: %u\n", *block_length);

    // Allocate memory for the block and read it
    *block = malloc(*block_length);
    if (*block == NULL) {
        perror("Memory allocation failed");
        return -1;
    }

    uint32_t total_received = 0;
    while (total_received < *block_length) {
        bytes_received = recv(sockfd, *block + total_received, *block_length - total_received, 0);
        if (bytes_received < 0) {
            perror("Failed to read the block");
            free(*block);
            return -1;
        }
        total_received += bytes_received;
    }

    printf("bytes_received: %u\n", total_received);

    return 0;
}



char *download_piece(int sockfd, uint32_t piece_index, uint32_t piece_length) {
    // Get bitfield message
    char *bitfield_message = NULL;
    if (read_packet(sockfd, &bitfield_message) < 1 || bitfield_message[0] != BITFIELD) {
        perror("Failed to recv bitfield packet");
        free(bitfield_message);
        exit(1);
    }
    free(bitfield_message);

    // Send interested message
    char interested_message[LENGTH_PREFIX_SIZE + 1];
    uint32_t length_prefix = htonl(1);
    memcpy(interested_message, &length_prefix, LENGTH_PREFIX_SIZE);
    interested_message[LENGTH_PREFIX_SIZE] = INTERESTED;

    if (send(sockfd, interested_message, LENGTH_PREFIX_SIZE + 1, 0) != LENGTH_PREFIX_SIZE + 1) {
        perror("Failed to send interested packet");
        exit(1);
    }

    // Get unchoke message
    char *unchoke_message = NULL;
    if (read_packet(sockfd, &unchoke_message) < 1 || unchoke_message[0] != UNCHOKE) {
        perror("Failed to recv unchoke packet");
        free(unchoke_message);
        exit(1);
    }
    free(unchoke_message);

    // Calculate the number of blocks in the piece
    uint32_t num_blocks = (piece_length + BLOCK_LENGTH - 1) / BLOCK_LENGTH;

    // Buffer to hold the entire piece
    char *piece_data = malloc(piece_length);
    if (!piece_data) {
        perror("Memory allocation failed for piece data");
        exit(1);
    }
    
    printf("piece length: %u\n", piece_length);

    // Download each block
    for (uint32_t block_index = 0; block_index < num_blocks; ++block_index) {
        uint32_t begin = block_index * BLOCK_LENGTH;
        uint32_t request_length = (begin + BLOCK_LENGTH > piece_length) ? (piece_length - begin) : BLOCK_LENGTH;

        // Construct request message
        char request_packet[17];
        construct_request_message(request_packet, piece_index, begin, request_length);

        // Send request packet
        if (send(sockfd, request_packet, 17, 0) != 17) {
            perror("Failed to send request packet");
            free(piece_data);
            exit(1);
        }

        // Receive piece message
        uint32_t index, piece_begin, block_length;
        char *block = NULL;
        if (read_piece_message(sockfd, &index, &piece_begin, &block, &block_length) < 0) {
            perror("Failed to receive piece message");
            free(piece_data);
            exit(1);
        }

        // Copy the received block to the correct position in the piece buffer
        memcpy(piece_data + piece_begin, block, block_length);
        free(block);

        printf("Received block: index=%u, begin=%u, block_length=%u\n", index, piece_begin, block_length);
    }

    return piece_data;  // Return the entire piece data
}

int verify_piece(const char *piece_recived, size_t piece_length, const unsigned char *piece_hash){
    unsigned char new_piece_hash[SHA1_DIGEST_LENGTH];
    if (!sha1_hash((const unsigned char *)piece_recived, (size_t)piece_length, new_piece_hash)) {
        return 0; // SHA1 hash calculation failed
    }

    return memcmp(new_piece_hash, piece_hash, SHA1_DIGEST_LENGTH) == 0;
}