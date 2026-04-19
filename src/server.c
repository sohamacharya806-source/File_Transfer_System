#include "../include/common.h"

/* ---------------------------------------------------------------
 * server.c — Persistent File-Transfer Server
 *
 * Protocol (per connection):
 *   1. Client sends 4-byte little-endian uint32 = filename length
 *   2. Client sends <filename> bytes  (just the basename, no path)
 *   3. Client sends raw file bytes until it closes the connection
 *
 * Server loops forever accepting connections and saving received
 * files under the original filename.
 * Type  "exit"  in this terminal and press Enter to quit.
 * --------------------------------------------------------------- */

#ifdef _WIN32
  #include <windows.h>
  DWORD WINAPI exit_watcher(LPVOID arg) {
      (void)arg;
      char line[64];
      while (fgets(line, sizeof(line), stdin)) {
          /* strip newline */
          size_t len = strlen(line);
          if (len > 0 && line[len-1] == '\n') line[--len] = '\0';
          if (len > 0 && line[len-1] == '\r') line[--len] = '\0';
          if (strcmp(line, "exit") == 0) {
              printf("[server] Shutting down...\n");
              exit(0);
          }
      }
      return 0;
  }
#endif

/* Receive exactly `n` bytes into `buf`. Returns 0 on success, -1 on failure. */
static int recv_all(SOCKET s, void *buf, int n) {
    int total = 0;
    char *p = (char*)buf;
    while (total < n) {
        int r = recv(s, p + total, n - total, 0);
        if (r <= 0) return -1;
        total += r;
    }
    return 0;
}

int main(void) {
    SOCKET server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    init_sockets();

    /* Create listening socket */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("[server] Socket creation failed.\n");
        return 1;
    }

    /* Allow port reuse so restarting doesn't fail */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("[server] Bind failed.\n");
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        printf("[server] Listen failed.\n");
        return 1;
    }

    printf("[server] Listening on port %d...\n", PORT);
    printf("[server] Type  exit  and press Enter to quit.\n\n");

#ifdef _WIN32
    /* Spawn a background thread that watches stdin for "exit" */
    HANDLE t = CreateThread(NULL, 0, exit_watcher, NULL, 0, NULL);
    if (t) CloseHandle(t);
#endif

    /*  Main accept loop  */
    while (1) {
        printf("[server] Waiting for connection...\n");

        SOCKET conn = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (conn == INVALID_SOCKET) {
            printf("[server] Accept failed. Retrying...\n");
            continue;
        }

        printf("[server] Client connected.\n");

        /*  Step 1: receive filename length (4 bytes, LE uint32)  */
        uint32_t name_len = 0;
        if (recv_all(conn, &name_len, 4) < 0 || name_len == 0 || name_len > 255) {
            printf("[server] Bad filename header. Skipping.\n");
            closesocket(conn);
            continue;
        }

        /*  Step 2: receive filename  */
        char filename[256] = {0};
        if (recv_all(conn, filename, (int)name_len) < 0) {
            printf("[server] Failed to receive filename. Skipping.\n");
            closesocket(conn);
            continue;
        }
        filename[name_len] = '\0';

        /* Prevent path traversal — keep only the basename */
        char *base = filename;
        for (char *p = filename; *p; ++p)
            if (*p == '/' || *p == '\\') base = p + 1;

        printf("[server] Receiving file: %s\n", base);

        /*  Step 3: open file and receive data */
        FILE *file = fopen(base, "wb");
        if (!file) {
            printf("[server] Cannot open '%s' for writing.\n", base);
            closesocket(conn);
            continue;
        }

        char buffer[BUFFER_SIZE];
        int bytes_read;
        long total_bytes = 0;
        while ((bytes_read = recv(conn, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes_read, file);
            total_bytes += bytes_read;
        }

        fclose(file);
        closesocket(conn);

        printf("[server] Saved '%s' (%ld bytes).\n\n", base, total_bytes);
    }

    /* Never reached, but tidy up */
    closesocket(server_fd);
    cleanup_sockets();
    return 0;
}
