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
#define BLOCK_LENGTH 16384

#define LENGTH_PREFIX_SIZE 4 
#define UNCHOKE 1            // no payload
#define INTERESTED 2          // no payload
#define BITFIELD 5    
#define REQUEST 6     
#define PIECE 7       

int create_socket();

// sends handshake packet to peer, get back a response (same format)
char *perform_peer_handshake(int sockfd, const unsigned char *info_hash, const char *peer_ip, int peer_port);

// handle peer messanging - recv bitfield, send intrested, recv unchoke, loop(send request, recv piece). return the contents of the piece (bytes)
char *download_piece(int sockfd, uint32_t piece_index, uint32_t piece_length);

// compares the hash of the piece we have gotten to the hash piece from the metainfo
int verify_piece(const char *piece_recived, size_t piece_length, const unsigned char *piece_hash);


#endif // PEER_H
