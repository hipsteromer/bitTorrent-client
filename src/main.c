#include "info.h"
#include "decode.h"
#include "tracker.h"
#include "peer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

void print_usage() {
    printw("Usage:\n");
    printw("  1. Decode bencoded string\n");
    printw("  2. Show torrent file info\n");
    printw("  3. List peers from torrent file\n");
    printw("  4. Download specific piece\n");
    printw("  5. Download entire file\n");
    printw("  6. Quit\n");
}

void ncurses_decode() {
    char encoded_str[1024];
    echo();
    printw("Enter bencoded string: ");
    getnstr(encoded_str, sizeof(encoded_str));
    noecho();

    DecodedValue decoded = decode_bencode(encoded_str);
    clear();
    printw("Decoded value:\n");
    char *decoded_result_str = decode_value_to_string(decoded);
    printw(decoded_result_str);
    printw("\nPress any key to continue...");
    free(decoded_result_str);
    free_decoded_value(decoded);
    getch();
}

void ncurses_info() {
    char file_name[256];
    echo();
    printw("Enter torrent file name: ");
    getnstr(file_name, sizeof(file_name));
    noecho();

    char *content = read_torrent_file(file_name);
    if (content == NULL) {
        printw("Failed to read torrent file: %s\n", file_name);
        printw("Press any key to continue...");
        getch();
        return;
    }

    MetaInfo info = info_extract(content);
    clear();
    char *info_str_result = meta_info_to_string(info);
    printw(info_str_result);
    free(info_str_result);
    free_info(info);
    free(content);
    printw("\nPress any key to continue...");
    getch();
}

void ncurses_peers() {
    char file_name[256];
    echo();
    printw("Enter torrent file name: ");
    getnstr(file_name, sizeof(file_name));
    noecho();

    char *content = read_torrent_file(file_name);
    if (content == NULL) {
        printw("Failed to read torrent file: %s\n", file_name);
        printw("Press any key to continue...");
        getch();
        return;
    }

    MetaInfo info = info_extract(content);
    PeersList peers = get_peers(info);
    clear();
    char *peers_str_result = peers_list_to_string(peers);
    printw(peers_str_result);
    free_peers(peers);
    free_info(info);
    free(content);
    printw("\nPress any key to continue...");
    getch();
}

void ncurses_download_piece() {
    int piece_index;
    char target_file[256], torrent_file[256];

    echo();
    printw("Enter piece index: ");
    scanw("%d", &piece_index);  // Read piece index as an integer
    printw("Enter target file: ");
    getnstr(target_file, sizeof(target_file));
    printw("Enter torrent file: ");
    getnstr(torrent_file, sizeof(torrent_file));
    noecho();

    // Read the torrent file content
    char *content = read_torrent_file(torrent_file);
    if (content == NULL) {
        printw("Failed to read torrent file: %s\n", torrent_file);
        printw("Press any key to continue...");
        getch();
        return;
    }

    // Extract metadata information from the torrent file
    MetaInfo info = info_extract(content);
    PeersList peers_list = get_peers(info);

    // Handshake with the proper peer
    int sockfd = create_socket();
    if (sockfd < 0) {
        free_peers(peers_list);
        free_info(info);
        free(content);
        return;
    }

    // Perform handshake with the peer
    char *response = perform_peer_handshake(sockfd, info.info_hash, peers_list.peers[0].ip, peers_list.peers[0].port);
    if (response == NULL) {
        printw("Failed to perform handshake with peer %s:%d\n", peers_list.peers[0].ip, peers_list.peers[0].port);
        close(sockfd);
        free_peers(peers_list);
        free_info(info);
        free(content);
        printw("Press any key to continue...");
        getch();
        return;
    }
    free(response);

    // Calculate the piece length
    uint32_t piece_length = (piece_index == info.num_pieces - 1) ? 
                            info.length - (piece_index * info.piece_length) : 
                            info.piece_length;

    // Download the specified piece
    char *piece_data = download_piece(sockfd, piece_index, piece_length);
    if (!verify_piece(piece_data, piece_length, info.pieces_hashes[piece_index])) {
        printw("Failed to verify piece\n");
        free(piece_data);
        free_peers(peers_list);
        free_info(info);
        free(content);
        close(sockfd);
        printw("Press any key to continue...");
        getch();
        return;
    }

    // Write the piece data to the target file
    FILE *file = fopen(target_file, "wb");
    if (file == NULL) {
        printw("Failed to open target file: %s\n", target_file);
        free(piece_data);
        free_peers(peers_list);
        free_info(info);
        free(content);
        close(sockfd);
        printw("Press any key to continue...");
        getch();
        return;
    }

    // Write the piece data to the file
    size_t bytes_written = fwrite(piece_data, 1, piece_length, file);
    if (bytes_written != piece_length) {
        printw("Failed to write the complete piece data to file\n");
        fclose(file);
        free(piece_data);
        free_peers(peers_list);
        free_info(info);
        free(content);
        close(sockfd);
        printw("Press any key to continue...");
        getch();
        return;
    }

    fclose(file);
    free(piece_data);
    free_peers(peers_list);
    free_info(info);
    free(content);
    close(sockfd);

    printw("Piece %d downloaded to %s\n", piece_index, target_file);
    printw("Press any key to continue...");
    getch();
}

void ncurses_download_file() {
    char target_file[256], torrent_file[256];

    echo();
    printw("Enter target file: ");
    getnstr(target_file, sizeof(target_file));
    printw("Enter torrent file: ");
    getnstr(torrent_file, sizeof(torrent_file));
    noecho();

    // Get peers' IP
    char *content = read_torrent_file(torrent_file);
    if (content == NULL) {
        printw("Failed to read torrent file: %s\n", torrent_file);
        printw("Press any key to continue...");
        getch();
        return;
    }
    MetaInfo info = info_extract(content);
    PeersList peers_list = get_peers(info);

    // Open the target file for writing
    FILE *file = fopen(target_file, "wb");
    if (file == NULL) {
        perror("Failed to open target file for writing");
        free_peers(peers_list);
        free_info(info);
        free(content);
        printw("Press any key to continue...");
        getch();
        return;
    }

    for (uint32_t piece_index = 0; piece_index < info.num_pieces; piece_index++) {
        uint32_t piece_length = (piece_index == info.num_pieces - 1) ? 
                                info.length - (piece_index * info.piece_length) : 
                                info.piece_length;

        char *piece_data = NULL;
        int sockfd = -1;
        for (int peer_index = 0; peer_index < peers_list.count; peer_index++) {
            sockfd = create_socket();
            if (sockfd < 0) continue;

            char *response = perform_peer_handshake(sockfd, info.info_hash, peers_list.peers[peer_index].ip, peers_list.peers[peer_index].port);
            if (response == NULL) {
                close(sockfd);
                continue;
            }

            piece_data = download_piece(sockfd, piece_index, piece_length);
            free(response);
            close(sockfd);

            if (piece_data != NULL && verify_piece(piece_data, piece_length, info.pieces_hashes[piece_index])) {
                break;
            }

            free(piece_data);
            piece_data = NULL;
        }

        if (piece_data == NULL) {
            printw("Failed to download piece %u\n", piece_index);
            fclose(file);
            free_peers(peers_list);
            free_info(info);
            free(content);
            printw("Press any key to continue...");
            getch();
            return;
        }

        fwrite(piece_data, 1, piece_length, file);
        free(piece_data);
    }

    fclose(file);
    free_peers(peers_list);
    free_info(info);
    free(content);
    printw("File downloaded successfully\n");
    printw("Press any key to continue...");
    getch();
}

int main(int argc, char *argv[]) {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    int choice;
    while (1) {
        clear();
        print_usage();
        choice = getch();

        switch (choice) {
            case '1':
                clear();
                ncurses_decode();
                break;
            case '2':
                clear();
                ncurses_info();
                break;
            case '3':
                clear();
                ncurses_peers();
                break;
            case '4':
                clear();
                ncurses_download_piece();
                break;
            case '5':
                clear();
                ncurses_download_file();
                break;
            case '6':
                endwin();
                return 0;
            default:
                printw("Invalid choice, press any key to continue...");
                getch();
                break;
        }
    }

    endwin();
    return 0;
}
