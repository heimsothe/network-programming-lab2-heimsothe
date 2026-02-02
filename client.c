/* ================================================================
 * client.c — UDP Client for JSON Object Transmission
 * 
 * Author: Elijah Heimsoth
 * Date: 02-01-2026
 *
 * Reads a custom data file containing key:value pairs, constructs
 * JSON objects using the cJSON library, serializes them to JSON
 * strings, and sends them to a UDP server.
 *
 * Usage: ./client <ipaddr> <port>
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

/* 
 * Constants:
 * Maximum bytes for a single key or value token during parsing.
 */
#define MAX_TOKEN 1024

// Function prototypes

/*
 * makeSocket():
 * Creates a UDP socket and fills in the server address structure.
 * Receives the port number and a server_address struct with IP address
 * field already validated/set by main().
 */
void makeSocket(int *sd, int port, struct sockaddr_in *server_address);

/*
 * openFile():
 * Prompts the user for a filename and returns an open FILE pointer.
 * Re-prompts on invalid filenames.
 */
FILE *openFile();

/*
 * parseLine():
 * Parses a line of space-separated key:value pairs into a cJSON object.
 * Returns NULL for empty/invalid lines.
 * Any error on the line discards the entire line.
 */
cJSON *parseLine(const char *line);

/* 
 * rtrim():
 * Strips trailing whitespace (including newline) from a string.
 * Used by openFile() to clean fgets() input.
 */
char *rtrim(char *s);

/* ================================================================
 * main() - Main function and orchestrator for Client
 * 
 * Flow:
 *  1. Validate arguments (need exactly 3: program, IP, port)
 *  2. Validate and store IP Address in server_address
 *  3. Validate port number (all digits, range 0-65535)
 *  4. Create socket and complete server address struct
 *  5. Open the data file
 * ================================================================
 */
int main(int argc, char *argv[]) {
    int sd; // Socket descriptor
    struct sockaddr_in server_address; // Server address structure for sendto()
    int portNumber; // Port number (converted from string)
    
    // Step 1: Validate arguments (need exactly 3: program, IP, port)
    if (argc < 3) {
        printf("Error: Usage is client <ipaddr> <portnumber>\n");
        exit(1);
    }

    // Step 2: Validate and store IP Address in server_address
    if (!inet_pton(AF_INET, argv[1], &server_address.sin_addr)) {
        printf("Error: Bad IP address\n");
        printf("Valid IP Range: 0.0.0.0 - 255.255.255.255\n");
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

    // Step 4: Create socket and complete server address struct
    makeSocket(&sd, portNumber, &server_address);

    printf("Socket created, server address set to %s:%d\n", argv[1], portNumber);

    // Step 5: Open the data file
    FILE *fptr = openFile();
    
    printf("File opened successfully\n");

    // Clean up and exit
    fclose(fptr);
    close(sd);
    return 0;
}

/* ================================================================
 * makeSocket() — Create UDP socket and complete server address struct
 * 
 * This function:
 *  1. Creates a UDP socket
 *  2. Completes the server address struct by setting the address family and port number
 * 
 * Note: the address (sin_addr) was already set by main()
 * ================================================================
 */
void makeSocket(int *sd, int port, struct sockaddr_in *server_address) {
    *sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*sd == -1) {
        perror("socket");
        exit(1);
    }
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    // sin_addr was already set by main()
}

/* ================================================================
 * openFile() — Prompt for filename, open and return FILE*
 * 
 * This function:
 *  1. 
 * ================================================================
 */
FILE *openFile() {
    FILE *fptr = NULL;
    char fileName[100]; // Buffer for user input (filename)

    while (1) {
        memset(fileName, 0, 100);
        printf("What is the name of the data file? ");

        if (fgets(fileName, sizeof(fileName), stdin) == NULL) {
            perror("fgets");
            exit(1);
        }

        rtrim(fileName);

        if (strlen(fileName) == 0) {
            printf("Error: Must enter a filename.\n");
            continue;
        }

        fptr = fopen(fileName, "r");
        if (fptr == NULL) {
            printf("Error: Could not open file %s\n", fileName);
            continue;
        }

        return fptr;
    }
}

/* ================================================================
 * parseLine() — Stateful tokenizer for key:value pairs
 * ================================================================
 */
cJSON *parseLine(const char *line) {
    // TEMPORARY IMPLEMENTATION
    printf("parseLine() skeleton compiled successfully\n");
    return NULL;
}

/* ================================================================
 * rtrim() — Strip trailing whitespace
 * 
 * This function:
 *  1. Strips trailing whitespace (including newline) from a string
 *  2. Is used by openFile() to clean fgets() input
 * ================================================================
 */
char *rtrim(char *s) {
    char *back;

    // Guards against NULL or empty strings
    if (s == NULL || *s == '\0') {
        return s;
    }

    // Start at the last character of the string before the null terminator
    back = s + strlen(s) - 1;

    // Walk backwards while the character is whitespace
    while (back >= s && isspace((unsigned char)*back)) {
        back--;
    }

    // Place null terminator after the last non-whitespace character
    *(back + 1) = '\0';

    // Return the trimmed string
    return s;
}