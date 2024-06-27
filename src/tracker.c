#include "tracker.h"

const char *peer_id = "00112233445566778899";
const char *port = "6881";
const char uploaded  = '0';
const char downloaded = '0';
const char compact ='1';

PeersList get_peers(MetaInfo info)
{
    CURL *curl;
    CURLcode result;

    curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "HTTP request failed\n");
        exit(1);
    }

    Response response;
    response.string = malloc(1);
    response.size = 0;

    // URL setup
    char *safe_info_hash = curl_easy_escape(curl, info.info_hash, 20); // Info hash is 20 bytes long

    // Calculate the length of the URL string
    size_t url_length = snprintf(NULL, 0, "%s?peer_id=%s&info_hash=%s&port=%s&left=%zu&downloaded=%c&uploaded=%c&compact=%c", 
                                info.url, peer_id, safe_info_hash, port, info.length, downloaded, uploaded, compact);
    char *url = malloc(url_length + 1); // Add 1 for null terminator

    // Construct the URL string
    snprintf(url, url_length + 1, "%s?peer_id=%s&info_hash=%s&port=%s&left=%zu&downloaded=%c&uploaded=%c&compact=%c", 
            info.url, peer_id, safe_info_hash, port, info.length, downloaded, uploaded, compact);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);

    result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        fprintf(stderr, "Error : %s\n", curl_easy_strerror(result));
        exit(1);
    }

    DecodedValue decodedResponse = decode_bencode(response.string);
    int peers_index = find_index(decodedResponse, "peers");

    if(peers_index == -1) {
        fprintf(stderr, "peers key not found");
        exit(1);
    }
    size_t raw_addresses_string_length = decodedResponse.val.dict[peers_index].val.val.length;
    char *raw_addresses_string = malloc(raw_addresses_string_length);
    memcpy(raw_addresses_string, decodedResponse.val.dict[peers_index].val.val.str, raw_addresses_string_length);

    // Parse peers
    PeersList peersList;
    peersList.count = raw_addresses_string_length / 6;
    peersList.peers = malloc(peersList.count * sizeof(Peer));

    for (size_t i = 0; i < peersList.count; ++i) {
        struct in_addr ip_addr;
        memcpy(&ip_addr, &raw_addresses_string[i * 6], 4);
        inet_ntop(AF_INET, &ip_addr, peersList.peers[i].ip, INET_ADDRSTRLEN);
        uint16_t port;
        memcpy(&port, &raw_addresses_string[i * 6 + 4], 2);
        peersList.peers[i].port = ntohs(port);
    }

    curl_easy_cleanup(curl);
    free(url); // Free the allocated memory for the URL
    curl_free(safe_info_hash); // Free the escaped info_hash
    free(response.string);
    free(raw_addresses_string);

    return peersList;
}

size_t write_chunk(void *data, size_t size, size_t nmemb, void *userdata) {
    size_t real_size = size * nmemb;

    Response *response = (Response *) userdata;
    char *ptr = realloc(response->string, response->size + real_size + 1);
    if(ptr == NULL) {
        fprintf(stderr, "Memory allocation failed");
        return CURLE_WRITE_ERROR;
    }

    response->string = ptr;
    memcpy(&(response->string[response->size]), data, real_size);
    response->size += real_size;
    response->string[response->size] = '\0';

    return real_size;
}

void print_peers(PeersList peers) {
    for (size_t i = 0; i < peers.count; ++i) {
        printf("%s:%d\n", peers.peers[i].ip, peers.peers[i].port);
    }
}

char* peers_list_to_string(PeersList peers) {
    char *result = NULL;
    size_t result_size = 0;
    char buffer[256];

    for (size_t i = 0; i < peers.count; ++i) {
        size_t peer_info_size = snprintf(buffer, sizeof(buffer), "%s:%d\n", peers.peers[i].ip, peers.peers[i].port);
        result_size += peer_info_size;

        result = (char*)realloc(result, result_size + 1);
        strcat(result, buffer);
    }

    return result;
}

void free_peers(PeersList peers) {
    free(peers.peers);
}
