/* ================================================================
 * server.c — UDP Multicast Server for JSON Object Reception
 *
 * Author: Elijah Heimsoth
 * Date: 02-13-2026
 *
 * Joins a multicast group and receives serialized JSON strings over
 * UDP, deserializes them with cJSON, and prints each key-value pair.
 *
 * Runs continuously until Ctrl+C.
 *
 * Usage: ./server <multicast_ip> <port>
 * Example: ./server 239.0.0.1 5000
 * ================================================================
 */

 // C library headers
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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

// Function prototypes

/* ================================================================
 * joinMulticastGroup():
 * Join a UDP socket to a multicast group
 *
 * Parameters:
 *  - sd: socket descriptor
 *  - multicastIP: multicast group IP address
 *
 * Returns:
 *  - 0 on success
 *  - -1 on error (perror)
 * ================================================================ */
int joinMulticastGroup(int sd, const char *multicastIP);

/* ================================================================
 * main():
 * Main function and orchestrator for Server
 * 
 * Flow:
 *  1. Validate arguments (IP, multicast range, port)
 *  2. Create socket and bind to port
 *  3. Join the multicast group
 *  4. Receive loop
 *  5. Cleanup
 * ================================================================ */
int main(int argc, char *argv[]) {
    int sd; // Socket descriptor
    struct sockaddr_in server_address; // Server address
    int portNumber; // Port number

    printf("========================SETUP========================\n");

    /*
     * Step 1: Validate arguments (IP, multicast range, port)
     *
     * validateArguments() checks argc, validates the IP format with
     * inet_pton(), verifies the multicast range, and parses the port.
     * 
     * On success, the binary IP is written into a temporary in_addr.
     * The server binds to INADDR_ANY, not the multicast IP. The 
     * multicast IP is only used when joining the group via setsockopt.
     */
    struct in_addr multicast_check;
    validateArguments(argc, argv, &multicast_check, &portNumber);

    // Step 2: Create socket and bind to port
    setupSocket(&sd, portNumber, &server_address, MODE_SERVER);

    // Step 3: Join the multicast group
    if (joinMulticastGroup(sd, argv[1]) == -1) {
        printf("Error: Failed to join multicast group %s\n", argv[1]);
        close(sd);
        exit(1);
    }

    printf("Socket created, joined multicast group %s on port %d...\n",
           argv[1], portNumber);
    printf("=====================================================\n\n");

    // Step 4: Receive loop
    char buffer[BUFFER_SIZE]; // Buffer for incoming data
    struct sockaddr_in client_address; // Client address (filled by recvfrom)
    socklen_t addr_len; // Length of client address
    int bytesReceived; // Return value from recvfrom
    char clientIP[INET_ADDRSTRLEN]; // IP address of client

    while (1) {
        // Reset addr_len before EACH recvfrom() call.
        addr_len = sizeof(client_address);

        // Receive data from client
        bytesReceived = recvfrom(sd, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr *)&client_address, &addr_len);
        
        if (bytesReceived == -1) {
            perror("recvfrom");
            continue;
        }

        // Null terminate buffer
        buffer[bytesReceived] = '\0';

        // Convert client binary IP to string
        inet_ntop(AF_INET, &client_address.sin_addr, clientIP, INET_ADDRSTRLEN);

        printf("Received from %s:%d\n", clientIP, ntohs(client_address.sin_port));
        printf("=====================================================\n");
        
        // Parse the JSON string into a cJSON object
        cJSON *json = cJSON_Parse(buffer);
        if (json == NULL) {
            printf("Invalid JSON received: %s\n", buffer);
            printf("=====================================================\n\n");
            continue;
        }

        // Print the parsed JSON
        printJSONObject(json, MODE_SERVER, 0);

        // Free the cJSON object tree memory allocation
        cJSON_Delete(json);

        printf("=====================================================\n\n");
    }
    
    // Cleanup (Unreachable code)
    close(sd);
    return 0;
}

/* ================================================================
 * joinMulticastGroup() — Join a UDP socket to a multicast group
 *
 * Parameters:
 *  - sd: socket descriptor
 *  - multicastIP: multicast group IP address
 *
 * Returns:
 *  - 0 on success
 *  - -1 on error (perror)
 * ================================================================ */
int joinMulticastGroup(int sd, const char *multicastIP) {
    struct ip_mreq mreq;

    // Zero out multicast request structure
    memset(&mreq, 0, sizeof(mreq));

    /*
     * Fill in the multicast request structure
     *  - imr_multiaddr: multicast group IP address
     *  - imr_interface: which network interface to use
     */
    int result = inet_pton(AF_INET, multicastIP, &mreq.imr_multiaddr);
    if (result == 0) {
        fprintf(stderr, "joinMulticastGroup: invalid IP address format: %s\n",
                multicastIP);
        return -1;
    } else if (result == -1) {
        perror("inet_pton");
        return -1;
    }
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    /*
     * Join the multicast group
     * 
     * - IPPROTO_IP: IP Proctocol IPv4 level option
     * - IP_ADD_MEMBERSHIP: Add multicast membership option
     */
    if (setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) == -1) {
        perror("setsockopt IP_ADD_MEMBERSHIP");
        return -1;
    }

    return 0;
}