#ifndef TRACKER_H
#define TRACKER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include "decode.h"
#include "info.h"

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
} Peer;

typedef struct {
    Peer *peers;
    size_t count;
} PeersList;

typedef struct {
    char *string;
    size_t size;
} Response;

// return a list of peers addresses
PeersList get_peers(MetaInfo info);

// writes incoming data to a string
size_t write_chunk(void *data, size_t size, size_t nmemb, void *userdata);

// print peers addresses list (useless with ncurses)
void print_peers(PeersList peers);

// free the memory allocated for the peers addresses list
void free_peers(PeersList peers);

// constucts a string of the peer addresses list (useful for ncurses)
char* peers_list_to_string(PeersList peers);

#endif // TRACKER_H
