/* ================================================================
 * utils.c — Shared Utility Functions
 *
 * Common functions used by both client and server programs.
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../cJSON.h"
#include "utils.h"

/* ================================================================
 * printJSONObject() — Display all key-value pairs in a cJSON object
 *
 * Handles three JSON value types:
 *  - Strings: printed as-is (quoted strings retain their quotes)
 *  - Booleans: printed as "true" or "false"
 *  - Numbers: printed using %g (auto-formats integers and floats)
 *
 * Output format depends on the format parameter:
 *   FORMAT_CLIENT: Simple left-aligned "key: value"
 *   FORMAT_SERVER: Right-aligned columns (20 char minimum each)
 * ================================================================ */
void printJSONObject(cJSON *obj, OutputFormat format) {
    // Verify valid cJSON object.
    if (obj == NULL || !cJSON_IsObject(obj)) {
        printf("Error: Invalid JSON object\n");
        return;
    }

    if (format == FORMAT_DEBUG) {
        printf("DEBUG MODE:\n");
        printf("%s\n", cJSON_Print(obj));
        return;
    }

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, obj) {
        /*
         * Print key based on format:
         *   FORMAT_SERVER: Right-aligned, 20-char width
         *   FORMAT_CLIENT: Left-aligned, no padding
         */
        if (format == FORMAT_SERVER) {
            printf("%20s: ", item->string);
        }
        else if (format == FORMAT_CLIENT) {
            printf("Parsed JSON data:\n");
            printf("%s: ", item->string);
        }

        /*
         * Print value based on cJSON type:
         *   - String: use valuestring directly
         *   - Boolean: print "true" or "false" based on cJSON_IsTrue()
         *   - Number: use %g format (auto handles integers and floats)
         */
        if (cJSON_IsString(item)) {
            if (format == FORMAT_SERVER) {
                printf("%20s\n", item->valuestring);
            }
            else {
                printf("%s\n", item->valuestring);
            }
        }
        else if (cJSON_IsBool(item)) {
            const char *boolStr = cJSON_IsTrue(item) ? "true" : "false";
            if (format == FORMAT_SERVER) {
                printf("%20s\n", boolStr);
            }
            else {
                printf("%s\n", boolStr);
            }
        }
        else if (cJSON_IsNumber(item)) {
            if (format == FORMAT_SERVER) {
                printf("%20g\n", item->valuedouble);
            }
            else {
                printf("%g\n", item->valuedouble);
            }
        }
    }
}

/* ================================================================
 * setupSocket() — Create and configure a UDP socket
 *
 * Behavior depends on mode:
 *   SOCKET_CLIENT: Creates socket, sets sin_family and sin_port.
 *                  Does NOT set sin_addr (caller sets via inet_pton).
 *                  Does NOT bind (OS assigns ephemeral port on sendto).
 *
 *   SOCKET_SERVER: Creates socket, sets SO_REUSEADDR and SO_REUSEPORT,
 *                  sets sin_family, sin_port, sin_addr to INADDR_ANY,
 *                  then binds to the address.
 * ================================================================ */
void setupSocket(int *sd, int port, struct sockaddr_in *address, SocketMode mode) {
    // Create UDP socket
    *sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sd == -1) {
        perror("socket");
        exit(1);
    }

    // Set address family and port (common to both modes)
    address->sin_family = AF_INET;
    address->sin_port = htons(port);

    // Server-only: set socket options, set INADDR_ANY, and bind
    if (mode == SOCKET_SERVER) {
        int reuse = 1;
        setsockopt(*sd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        setsockopt(*sd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));

        address->sin_addr.s_addr = INADDR_ANY;

        if (bind(*sd, (struct sockaddr *)address, sizeof(*address)) == -1) {
            perror("bind");
            exit(1);
        }
    }
    // Client mode: sin_addr already set by caller via inet_pton()
}
