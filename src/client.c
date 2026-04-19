#include "../include/common.h"

/* ---------------------------------------------------------------
 * client.c — File-Transfer Client
 *
 * Usage:  client.exe <server_ip> <filepath>
 *
 * Protocol:
 *   1. Sends 4-byte network-order uint32 = filename length
 *   2. Sends filename bytes (basename only)
 *   3. Sends raw file bytes, then closes connection
 * --------------------------------------------------------------- */

static int send_all(SOCKET sock, const void *buf, int len) {
    int total = 0;
    const char *p = (const char *)buf;
    while (total < len) {
        int sent = send(sock, p + total, len - total, 0);
        if (sent <= 0) return -1;
        total += sent;
    }
    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <filepath>\n", argv[0]);
        return -1;
    }

    const char *server_ip = argv[1];
    const char *filepath  = argv[2];

    /* Extract basename from the provided path */
    const char *basename = filepath;
    for (const char *p = filepath; *p; ++p)
        if (*p == '/' || *p == '\\') basename = p + 1;

    uint32_t name_len = (uint32_t)strlen(basename);
    if (name_len == 0 || name_len > 255) {
        printf("[client] Invalid filename.\n");
        return -1;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        printf("[client] Cannot open file '%s'.\n", filepath);
        return -1;
    }

    SOCKET sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    init_sockets();

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("[client] Socket creation error.\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(PORT);

#ifdef _WIN32
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
        printf("[client] Invalid address / Address not supported.\n");
        fclose(file);
        closesocket(sock);
        cleanup_sockets();
        return -1;
    }
#else
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("[client] Invalid address / Address not supported.\n");
        fclose(file);
        closesocket(sock);
        cleanup_sockets();
        return -1;
    }
#endif

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("[client] Connection Failed. Is the server running?\n");
        fclose(file);
        closesocket(sock);
        cleanup_sockets();
        return -1;
    }

    /* Step 1: send filename length (4-byte network-order uint32) */
    uint32_t name_len_net = htonl(name_len);
    if (send_all(sock, &name_len_net, sizeof(name_len_net)) < 0) {
        printf("[client] Failed to send filename length.\n");
        fclose(file);
        closesocket(sock);
        cleanup_sockets();
        return -1;
    }

    /* Step 2: send filename */
    if (send_all(sock, basename, (int)name_len) < 0) {
        printf("[client] Failed to send filename.\n");
        fclose(file);
        closesocket(sock);
        cleanup_sockets();
        return -1;
    }

    long total_bytes = 0;
    int bytes_read;
    while ((bytes_read = (int)fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send_all(sock, buffer, bytes_read) < 0) {
            printf("[client] Failed to send file data.\n");
            fclose(file);
            closesocket(sock);
            cleanup_sockets();
            return -1;
        }
        total_bytes += bytes_read;
    }

    printf("[client] Sent '%s' (%ld bytes) to %s successfully.\n", basename, total_bytes, server_ip);

    fclose(file);
    closesocket(sock);
    cleanup_sockets();
    return 0;
}
