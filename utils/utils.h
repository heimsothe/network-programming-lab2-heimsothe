/* ================================================================
 * utils.h — Shared Utility Functions
 *
 * Common functions used by both client and server programs.
 * ================================================================ */

#ifndef UTILS_H
#define UTILS_H

#include "../cJSON.h"
#include <netinet/in.h>  // struct sockaddr_in

/* ================================================================
 * ProgramMode enum:
 * Specifies the program mode for shared functions.
 * ================================================================ */
typedef enum {
    MODE_CLIENT,
    MODE_SERVER
} ProgramMode;

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
void validateArguments(int argc, char *argv[], struct in_addr *addr, int *port);

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
void printJSONObject(cJSON *obj, ProgramMode mode, int DebugMode);

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
void setupSocket(int *sd, int port, struct sockaddr_in *address, ProgramMode mode);

#endif /* UTILS_H */
