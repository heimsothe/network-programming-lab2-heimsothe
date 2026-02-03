/* ================================================================
 * server.c — UDP Server for JSON Object Reception
 *
 * Author: Elijah Heimsoth
 * Date: 02-03-2026
 *
 * Receives serialized JSON strings over UDP, deserializes them with
 * cJSON, and prints each key-value pair.
 * 
 * Runs continuously until Ctrl+C.
 *
 * Usage: ./server <port>
 * ================================================================
 */

 // C library headers
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

// Networking headers
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// cJSON library
#include "cJSON.h"

// Shared utilities
#include "utils/utils.h"

// Constants
#define BUFFER_SIZE 4096 // Maximum bytes for incoming UDP datagram

int main(int argc, char *argv[]) {
    int sd; // Socket descriptor
    struct sockaddr_in server_address; // Server address
    int portNumber; // Port number

    // Step 1: Validate arguments (need exactly 2: program, port)
    if (argc < 2) {
        printf("Error: Usage is server <portnumber>\n");
        exit(1);
    }

    // Step 2: Validate port number (all digits, range 0-65535)
    for (size_t i = 0; i < strlen(argv[1]); i++) {
        if (!isdigit(argv[1][i])) {
            printf("Error: The port number isn't a number\n");
            exit(1);
        }
    }
    portNumber = strtol(argv[1], NULL, 10);
    if (portNumber < 0 || portNumber > 65535) {
        printf("Error: Invalid port number\n");
        printf("Valid Port Range: 0-65535\n");
        exit(1);
    }

    printf("Port validated: %d\n", portNumber);

    return 0;
}