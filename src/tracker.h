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

char *url_encode(const unsigned char *data, size_t length);
char *construct_tracker_url(const char *base_url, const char *info_hash, const char *peer_id, const char *port, size_t length);
PeersList get_peers(MetaInfo info);
size_t write_chunk(void *data, size_t size, size_t nmemb, void *userdata);
void print_peers(PeersList peers);
void free_peers(PeersList peers);

#endif // TRACKER_H
