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
    printf("Server first test - not yet implemented\n");
    return 0;
}