/* ================================================================
 * utils.h — Shared Utility Functions
 *
 * Common functions used by both client and server programs.
 * ================================================================ */

#ifndef UTILS_H
#define UTILS_H

#include "cJSON.h"
#include <netinet/in.h>  // struct sockaddr_in

/* ----------------------------------------------------------------
 * OutputFormat enum:
 * Specifies the output format for printJSONObject().
 * ---------------------------------------------------------------- */
typedef enum {
    FORMAT_CLIENT,  // Simple "key: value" format
    FORMAT_SERVER   // Right-aligned columns (20 char minimum each)
} OutputFormat;

/* ----------------------------------------------------------------
 * SocketMode enum:
 * Specifies the socket setup mode for setupSocket().
 * ---------------------------------------------------------------- */
typedef enum {
    SOCKET_CLIENT,  // Create socket, set family/port only (no bind)
    SOCKET_SERVER   // Create socket, set options, set INADDR_ANY, bind
} SocketMode;

/*
 * printJSONObject():
 * Iterates all children of a cJSON object and prints each key-value pair.
 * Output format depends on the format parameter.
 */
void printJSONObject(cJSON *obj, OutputFormat format);

/*
 * setupSocket():
 * Creates a UDP socket and configures address structure.
 * Behavior depends on mode:
 *   SOCKET_CLIENT: Creates socket, sets sin_family and sin_port.
 *                  Caller must set sin_addr before calling.
 *   SOCKET_SERVER: Creates socket, sets socket options (SO_REUSEADDR,
 *                  SO_REUSEPORT), sets sin_addr to INADDR_ANY, binds.
 */
void setupSocket(int *sd, int port, struct sockaddr_in *address, SocketMode mode);

#endif /* UTILS_H */
