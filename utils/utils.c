/* ================================================================
 * utils.c — Shared Utility Functions
 *
 * Common functions used by both client and server programs.
 * ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../cJSON.h"
#include "utils.h"

/* ================================================================
 * validateArguments(): 
 * Validates command-line arguments for multicast client/server programs.
 * 
 * Checks:
 *  1. Argument count (expects 3: program name, IP, port)
 *  2. IPv4 address format using inet_pton()
 *  3. Multicast range check (first octet must be 224-239)
 *  4. Port is numeric (all digits) and in valid range (0-65535)
 *
 * On any validation failure, prints a descriptive error and exits.
 * On success, the parsed IP is stored in *addr and port in *port.
 * ================================================================ */
void validateArguments(int argc, char *argv[], struct in_addr *addr, int *port) {

    // Step 1: Argument count check
    if (argc < 3) {
        printf("Error: Usage is %s <multicast_ip> <portnumber>\n", argv[0]);
        printf("Example: %s 239.0.0.1 5000\n", argv[0]);
        exit(1);
    }

    /*
     * Step 2: Validate IPv4 address format
     *
     * inet_pton() converts a IP string into binary network-byte-order,
     * and stores the result directly into struct in_addr.
     *
     * inet_pton() returns:
     *   1  = success (valid address for the given family)
     *   0  = input is not a valid address in the specified family
     *  -1  = address family not supported
     */
    if (inet_pton(AF_INET, argv[1], addr) != 1) {
        printf("Error: Invalid IP address format: %s\n", argv[1]);
        exit(1);
    }

    // Step 3: Verify the address falls within the multicast range
    unsigned char firstOctet = ((unsigned char *)&addr->s_addr)[0];
    if (firstOctet < 224 || firstOctet > 239) {
        printf("Error: Not a multicast address: %s\n", argv[1]);
        printf("Multicast range: 224.0.0.0 - 239.255.255.255\n");
        exit(1);
    }

    // Step 4: Validate port number format (must be all digits)
    for (size_t i = 0; i < strlen(argv[2]); i++) {
        if (!isdigit(argv[2][i])) {
            printf("Error: The port number isn't a number\n");
            exit(1);
        }
    }

    // Step 5: Convert port string to integer and validate range (0-65535)
    *port = strtol(argv[2], NULL, 10);
    if (*port < 0 || *port > 65535) {
        printf("Error: Invalid port number\n");
        printf("Valid Port Range: 0-65535\n");
        exit(1);
    }
}

/* ================================================================
 * printJSONObject(): 
 * Display all key-value pairs in a cJSON object.
 *
 * Handles three JSON value types:
 *  - Strings: printed as-is (quoted strings retain their quotes)
 *  - Booleans: printed as "true" or "false"
 *  - Numbers: printed using %g (auto-formats integers and floats)
 *
 * Output format depends on the format parameter:
 *  MODE_CLIENT: Simple left-aligned "key: value"
 *  MODE_SERVER: Right-aligned columns (20 char minimum each)
 * 
 * DebugMode:
 *  0: Normal mode - prints JSON object in specified mode format
 *  1: Debug mode - prints JSON object in cJSON_Print format
 * ================================================================ */
void printJSONObject(cJSON *obj, ProgramMode mode, int DebugMode) {
    // Verify valid cJSON object.
    if (obj == NULL || !cJSON_IsObject(obj)) {
        printf("Error: Invalid JSON object\n");
        return;
    }

    if (DebugMode == 1) {
        printf("DEBUG MODE:\n");
        printf("%s\n", cJSON_Print(obj));
        return;
    }

    cJSON *item = NULL;
    cJSON_ArrayForEach(item, obj) {
        /*
         * Print key based on format:
         *  FORMAT_SERVER: Right-aligned, 20-char width
         *  FORMAT_CLIENT: Left-aligned, no padding
         */
        if (mode == MODE_SERVER) {
            printf("%20s: ", item->string);
        }
        else if (mode == MODE_CLIENT) {
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
            if (mode == MODE_SERVER) {
                printf("%20s\n", item->valuestring);
            }
            else {
                printf("%s\n", item->valuestring);
            }
        }
        else if (cJSON_IsBool(item)) {
            const char *boolStr = cJSON_IsTrue(item) ? "true" : "false";
            if (mode == MODE_SERVER) {
                printf("%20s\n", boolStr);
            }
            else {
                printf("%s\n", boolStr);
            }
        }
        else if (cJSON_IsNumber(item)) {
            if (mode == MODE_SERVER) {
                printf("%20g\n", item->valuedouble);
            }
            else {
                printf("%g\n", item->valuedouble);
            }
        }
    }
}

/* ================================================================
 * setupSocket(): 
 * Create and configure a UDP socket
 *
 * Behavior depends on mode:
 *  MODE_CLIENT: Creates socket, sets sin_family and sin_port.
 *               Does NOT set sin_addr (caller sets via inet_pton).
 *               Does NOT bind (OS assigns ephemeral port on sendto).
 *
 *  MODE_SERVER: Creates socket, sets SO_REUSEADDR and SO_REUSEPORT,
 *               sets sin_family, sin_port, sin_addr to INADDR_ANY,
 *               then binds to the address.
 * ================================================================ */
void setupSocket(int *sd, int port, struct sockaddr_in *address, ProgramMode mode) {
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
    if (mode == MODE_SERVER) {
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
