/* ================================================================
 * server.c — UDP Multicast Server for JSON Object Reception
 *
 * Author: Elijah Heimsoth
 * Date: 02-03-2026
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

// Declare joinMulticastGroup function
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
int joinMulticastGroup(int sd, const char *multicastIP);

int main(int argc, char *argv[]) {
    int sd; // Socket descriptor
    struct sockaddr_in server_address; // Server address
    int portNumber; // Port number

    // Step 1: Validate arguments (need exactly 3: program, multicast_ip, port)
    printf("========================SETUP========================\n");
    if (argc < 3) {
        printf("Error: Usage is server <multicast_ip> <portnumber>\n");
        printf("Example: ./server 239.0.0.1 5000\n");
        exit(1);
    }

    // Step 2: Validate multicast IP address
    struct in_addr multicast_check;
    if (inet_pton(AF_INET, argv[1], &multicast_check) != 1) {
        printf("Error: Invalid IP address format: %s\n", argv[1]);
        exit(1);
    }

    // Validate the IP is in the multicast range (224.0.0.0 - 239.255.255.255)
    unsigned char firstOctet = ((unsigned char *)&multicast_check.s_addr)[0];
    if (firstOctet < 224 || firstOctet > 239) {
        printf("Error: Not a multicast address: %s\n", argv[1]);
        printf("Multicast range: 224.0.0.0 - 239.255.255.255\n");
        exit(1);
    }

    // Step 3: Validate port number (all digits, range 0-65535)
    for (size_t i = 0; i < strlen(argv[2]); i++) {
        if (!isdigit(argv[2][i])) {
            printf("Error: The port number isn't a number\n");
            exit(1);
        }
    }
    portNumber = strtol(argv[2], NULL, 10);
    if (portNumber < 0 || portNumber > 65535) {
        printf("Error: Invalid port number\n");
        printf("Valid Port Range: 0-65535\n");
        exit(1);
    }

    // Step 4: Create socket and bind to port
    setupSocket(&sd, portNumber, &server_address, SOCKET_SERVER);

    // Step 5: Join the multicast group
    if (joinMulticastGroup(sd, argv[1]) == -1) {
        printf("Error: Failed to join multicast group %s\n", argv[1]);
        close(sd);
        exit(1);
    }

    printf("Socket created, joined multicast group %s on port %d...\n",
           argv[1], portNumber);
    printf("=====================================================\n\n");

    // Step 6: Receive loop
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
        printJSONObject(json, FORMAT_SERVER);

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