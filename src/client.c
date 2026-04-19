#include "../include/common.h"

/* 
 client.c — File-Transfer Client

 Usage:  client.exe <server_ip> <filepath>

 Protocol:
1. Sends 4-byte little-endian uint32 = filename length
2. Sends filename bytes (basename only)
3. Sends raw file bytes, then closes connection
  */

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <filepath>\n", argv[0]);
        return -1;
    }

    const char *server_ip = argv[1];
    const char *filepath  = argv[2];

    /* Extract base name from the provided path */
    const char *basename = filepath;
    for (const char *p = filepath; *p; ++p)
        if (*p == '/' || *p == '\\') basename = p + 1;

    uint32_t name_len = (uint32_t)strlen(basename);
    if (name_len == 0 || name_len > 255) {
        printf("[client] Invalid filename.\n");
        return -1;
    }

    SOCKET sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    FILE *file;

    init_sockets();

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("[client] Socket creation error.\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("[client] Invalid address / Address not supported.\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("[client] Connection Failed. Is the server running?\n");
        return -1;
    }

    /* send filename length (4-byte LE uint32)  */
    send(sock, (const char*)&name_len, 4, 0);

    /*   send filename  */
    send(sock, basename, (int)name_len, 0);

    /* open file and send data  */
    file = fopen(filepath, "rb");
    if (!file) {
        printf("[client] Cannot open file '%s'.\n", filepath);
        closesocket(sock);
        cleanup_sockets();
        return -1;
    }

    long total_bytes = 0;
    int bytes_read;
    while ((bytes_read = (int)fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(sock, buffer, bytes_read, 0);
        total_bytes += bytes_read;
    }

    printf("[client] Sent '%s' (%ld bytes) to %s successfully.\n", basename, total_bytes, server_ip);

    fclose(file);
    closesocket(sock);
    cleanup_sockets();
    return 0;
}
