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

// Constants
#define MAX_TOKEN 1024 // Maximum bytes for a single key or value token during parsing.

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
 * printJSONObject():
 * Iterates all children of a cJSON object and prints each key-value pair.
 * Used to display JSON data before sending.
 */
void printJSONObject(cJSON *obj);

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
 *  6. Read loop: parse each line, serialize, send
 *  7. Cleanup and exit
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
    printf("========================SETUP========================\n");
    makeSocket(&sd, portNumber, &server_address);

    printf("Socket created, server address set to %s:%d\n", argv[1], portNumber);

    // Step 5: Open the data file
    FILE *fptr = openFile();
    
    printf("File opened successfully\n");
    printf("=====================================================\n\n");

    // Step 6: Read loop: parse each line, serialize, send
    char *line = NULL;
    size_t lineLen = 0;
    ssize_t lengthRead;
    int sentCount = 0;

    // getline() reads one line at a time
    while ((lengthRead = getline(&line, &lineLen, fptr)) != -1) {
        // Parse the line into a cJSON object
        cJSON *json = parseLine(line);
        if (json == NULL) {
            continue; // Skip invalid/empty lines
        }

        /*
         * Convert cJSON object to a JSON string.
         * This is what will be sent over the network.
         */
        char *jsonString = cJSON_Print(json);
        if (jsonString == NULL) {
            printf("Error: cJSON_Print() allocation failed, skipping\n");
            cJSON_Delete(json);
            continue;
        }

        // Print all key-value pairs.
        printJSONObject(json);
        printf("\n");

        // Send the JSON string
        int bytesSent = sendto(sd, jsonString, strlen(jsonString), 0,
                               (struct sockaddr *)&server_address,
                               sizeof(server_address));
        
        if (bytesSent == -1) {
            perror("sendto");
        }
        else {
            printf("Sent %d bytes to %s:%d\n", bytesSent, argv[1], portNumber);
            printf("\n");
            sentCount++;
        }

        // Clean up the cJSON object and the JSON string
        cJSON_Delete(json);
        cJSON_free(jsonString);

        // Wait for 0.5 seconds before sending the next JSON object
        usleep(500000);
    }

    // Clean up the line buffer
    free(line);

    printf("Done! Sent %d JSON objects.\n", sentCount);

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
 * 
 * This function tokenizes a line of text into key:value pairs.
 * For each pair, it:
 *  1. Extracts the key (everything before the ':')
 *  2. Extracts the value (quoted or unquoted)
 *  3. Adds the pair to a cJSON object
 * 
 * Validation:
 *  - If any key or value on the line fails validation, the ENTIRE
 *    line is discarded.
 *  - Keys must be non-empty with no whitespace or colons.
 *  - Values can be quoted or unquoted.
 *  - Unquoted values may not contain whitespace.
 *  - Quoted values keep their enclosing quotes as part of the value
 *  - Escaped characters are are valid and become plain characters
 *  - Example input:  msg:"hello \"world\""
 *  - Stored key: msg
 *  - Stored value: "hello "world""
 * 
 * Returns: cJSON* on success, NULL on empty/invalid lines
 * ================================================================
 */
cJSON *parseLine(const char *line) {
    const char *pos = line; // Current read position in the line
    char key[MAX_TOKEN]; // Buffer for the current key
    char value[MAX_TOKEN]; // Buffer for the current value
    int pairCount = 0; // Number of key:value pairs added

    // Step 1: Skip leading whitespace
    while (*pos && isspace((unsigned char)*pos)) {
        pos++;
    }
    
    // If the line is empty, return NULL
    if (*pos == '\0' || *pos == '\n') {
        return NULL;
    }

    // Step 2: Create empty cJSON object
    cJSON *obj = cJSON_CreateObject();
    if (obj == NULL) {
        return NULL;
    }

    // Step 3: Main parsing loop
    while (*pos != '\0' && *pos != '\n') {
        
        // Skip whitespace between pairs
        while (*pos && *pos != '\n' && isspace((unsigned char)*pos)) {
            pos++;
        }

        // Check if we've reached the end after skipping whitespace
        if (*pos == '\0' || *pos == '\n') {
            break;
        }

        /*
         * Extract key
         * Valid key characters:
         *  - anything except whitespace, ':', '"', '\\', '\0', and '\n'.
         */
        const char *keyStart = pos;
        while (*pos != '\0' && *pos != '\n' && *pos != ':' &&
               !isspace((unsigned char)*pos) && *pos != '"' && *pos != '\\') {
            pos++;
        }

        // Check what character stopped the key scan (only ':' is a valid stop)
        if (*pos != ':') {
            if (isspace((unsigned char)*pos)) {
                printf("Warning: whitespace in key, skipping line\n");
            }
            else if (*pos == '"') {
                printf("Warning: quote character in key, skipping line\n");
            }
            else if (*pos == '\\') {
                printf("Warning: backslash in key, skipping line\n");
            }
            else {
                printf("Warning: no colon found in token, skipping line\n");
            }

            cJSON_Delete(obj);
            return NULL;
        }

        // Calculate key length
        int keyLen = (int)(pos - keyStart);

        // Empty key — colon appeared at the start, like ":value"
        if (keyLen == 0) {
            printf("Warning: empty key found, skipping line\n");
            cJSON_Delete(obj);
            return NULL;
        }

        // Key exceeds buffer size
        if (keyLen >= MAX_TOKEN) {
            printf("Warning: key too long, skipping line\n");
            cJSON_Delete(obj);
            return NULL;
        }

        // Copy key into buffer and null-terminate
        memcpy(key, keyStart, keyLen);
        key[keyLen] = '\0';

        // Advance past the ':' delimiter
        pos++;

        // Check for whitespace after colon
        if (isspace((unsigned char)*pos)) {
            printf("Warning: whitespace after colon for key '%s', skipping line\n", key);
            cJSON_Delete(obj);
            return NULL;
        }

        /* 
         * Extract value
         * 
         * Two modes: Quoted and Unquoted
         *  - Quoted: may contain spaces and escapes
         *  - Unquoted: may not contain spaces or backslashes
         */
        int valueLen = 0;

        // Quoted mode: value starts with '"'
        if (*pos == '"') {
            // Store the opening quote as part of the value
            value[0] = '"';
            valueLen++;

            // Advance past the opening quote
            pos++;

            // Walk through the quoted content
            while (*pos != '\0' && *pos != '\n') {

                // Check for escape sequences
                if (*pos == '\\') {
                    char nextChar = *(pos + 1);

                    // Trailing backslash — no character to escape.
                    if (nextChar == '\0' || nextChar == '\n') {
                        printf("Warning: trailing backslash in quoted value, skipping line\n");
                        cJSON_Delete(obj);
                        return NULL;
                    }

                    // Determine the escaped character
                    char escaped;
                    switch (nextChar) {
                        case '"': /* \" -> " */
                            escaped = '"';
                            break;
                        case '\\': /* \\ -> \ */
                            escaped = '\\';
                            break;
                        case 'n': /* \n -> newline */
                            escaped = '\n';
                            break;
                        case 't': /* \t -> tab */
                            escaped = '\t';
                            break;
                        case 'r': /* \r -> carriage return */
                            escaped = '\r';
                            break;
                        default:
                            printf("Warning: unrecognized escape sequence '\\%c' in quoted value, skipping line\n", nextChar);
                            cJSON_Delete(obj);
                            return NULL;
                    }

                    // Check if the value is too long
                    if (valueLen >= MAX_TOKEN - 1) {
                        printf("Warning: value too long, skipping line\n");
                        cJSON_Delete(obj);
                        return NULL;
                    }
                    value[valueLen] = escaped;
                    valueLen++;
                    
                    // Skip both the backslash and the escape char
                    pos += 2;
                }

                else if (*pos == '"') {
                    // Closing quote — end of quoted value
                    break; // Exit the loop, store the end quote after the loop
                }

                else {
                    // Regular character — copy it to the value buffer
                    // Check if the value is too long
                    if (valueLen >= MAX_TOKEN - 1) {
                        printf("Warning: value too long, skipping line\n");
                        cJSON_Delete(obj);
                        return NULL;
                    }
                    value[valueLen] = *pos;
                    valueLen++;
                    pos++;
                }
            }

            // Check for unclosed quote
            if (*pos != '"') {
                printf("Warning: unclosed quote, skipping line\n");
                cJSON_Delete(obj);
                return NULL;
            }

            // Store the closing quote as part of the value
            if (valueLen >= MAX_TOKEN - 1) {
                printf("Warning: value too long, skipping line\n");
                cJSON_Delete(obj);
                return NULL;
            }

            // Store the closing quote as part of the value
            value[valueLen] = '"';
            valueLen++;
            value[valueLen] = '\0';

            // Advance past the closing quote
            pos++;   
        }

        // Unquoted mode
        else {
            const char *valueStart = pos;

            /*
             * Walk through the unquoted content
             * Stopping at whitespace, newline, or backslash, or end of string.
             */
            while (*pos != '\0' && *pos != '\n' &&
                   !isspace((unsigned char)*pos) && *pos != '\\') {
                pos++;    
            }

            // Backslash in unquoted value — not allowed.
            if (*pos == '\\') {
                printf("Warning: backslash in unquoted value for key '%s', skipping line\n", key);
                cJSON_Delete(obj);
                return NULL;
            }

            valueLen = (int)(pos - valueStart);
            
            // Empty value — colon with nothing after it, like "key:"
            if (valueLen == 0) {
                printf("Warning: empty value for key '%s', skipping line\n", key);
                cJSON_Delete(obj);
                return NULL;
            }

            // Value exceeds buffer size
            if (valueLen >= MAX_TOKEN) {
                printf("Warning: value too long, skipping line\n");
                cJSON_Delete(obj);
                return NULL;
            }

            // Copy value into buffer and null-terminate
            memcpy(value, valueStart, valueLen);
            value[valueLen] = '\0';
        }

        // Add the key:value pair to the cJSON object
        cJSON_AddStringToObject(obj, key, value);
        pairCount++;
    }

    // If no pairs were successfully parsed, discard the object.
    if (pairCount == 0) {
        cJSON_Delete(obj);
        return NULL;
    }

    return obj;
}

/* ================================================================
 * printJSONObject() — Custom display of key:value pairs in a cJSON object
 *
 * This function iterates through a cJSON object and prints each
 * key:value pair
 * ================================================================
 */
void printJSONObject(cJSON *obj) {
    // Verify valid cJSON object.
    if (obj == NULL || !cJSON_IsObject(obj)) {
        printf("Error: Invalid JSON object\n");
        return;
    }

    printf("Parsed JSON data:\n");

    /*
     * Iterate through all key-value pairs in the object.
     *
     * cJSON represents objects as a linked list:
     *   obj->child points to the first item
     *   item->next points to the next sibling
     *
     * cJSON_ArrayForEach handles this traversal
     */
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, obj) {
        /*
         * item->string contains the key name
         * item->valuestring contains the string value
         */
        if (cJSON_IsString(item)) {
            printf("%s: %s\n", item->string, item->valuestring);
        }
    }
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