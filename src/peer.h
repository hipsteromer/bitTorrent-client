#ifndef PEER_H
#define PEER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define PROTOCOL_STRING "BitTorrent protocol"
#define PEER_ID "00112233445566778899"
#define PACKET_LENGTH 68

void construct_handshake_packet(char *handshake_packet, const char *info_hash);
int create_socket();
int connect_to_peer(int sockfd, const char *peer_ip, int peer_port);
int send_handshake(int sockfd, const char *handshake_packet);
int receive_response(int sockfd, char *response, size_t response_size);

#endif // PEER_H
