# Lab 2 - JSON Object Transmission

- **Author:** Elijah Heimsoth
- **Course:** Network Programming 3762
- **Date:** 02-13-2026

## Overview

This lab implements a UDP multicast client-server system for transmitting serialized JSON objects.

The client reads key-value pair data from a user-specified file, constructs JSON objects using the cJSON library, serializes them to JSON strings, and sends them to a multicast group over UDP.

The server joins the multicast group, receives the serialized JSON strings, deserializes them back into JSON objects, and displays each key-value pair in a formatted, human-readable output. Multiple servers can join the same multicast group to receive messages simultaneously.

## Building

```bash
make            # Build both client and server
make client     # Build only the client
make server     # Build only the server
make clean      # Remove compiled binaries
```

Compiled with `gcc -g -Wall` for debug symbols and all warnings enabled.

## Running

### 1. Start the server

```bash
./server <multicast_ip> <port>
```

Example:

```bash
./server 239.0.0.1 5000
```

- `multicast_ip` must be in the multicast range: 224.0.0.0–239.255.255.255
- `port` must be between 0 and 65535

The server joins the specified multicast group, then runs continuously until interrupted with Ctrl+C. Multiple servers can join the same group to receive messages simultaneously.

### 2. Start the client

```bash
./client <multicast_ip> <port>
```

Example:

```bash
./client 239.0.0.1 5000
```

- `multicast_ip` must be in the multicast range: 224.0.0.0–239.255.255.255

The client will prompt for the name of a message file (e.g., `sample.txt`). It reads the file line by line, parses each line into a JSON object, and sends the serialized JSON to the multicast group.

## Message Format

### Input

Each line contains space-separated key:value pairs. Values can be unquoted or enclosed in double quotes:

```
File_Name:"Presentation4.otp" File_Size:4MB File_Type:"Presentation" Date_Created:"2024-07-05" Description:"Webinar slide deck"
```

Value types are automatically detected:
- **Quoted values:** Treated as strings (quotes preserved)
- **Booleans:** `true` or `false` (case-insensitive)
- **Numbers:** Integers, floats, and scientific notation
- **Other:** Treated as strings

Escape sequences supported in quoted values: `\"`, `\\`, `\n`, `\t`, `\r`

### Output

The server prints each key-value pair with right-aligned 20-character columns:

```
Received from 127.0.0.1:65225
=====================================================
           File_Name:  "Presentation4.otp"
           File_Size:                  4MB
           File_Type:       "Presentation"
        Date_Created:         "2024-07-05"
         Description: "Webinar slide deck"
=====================================================
```

## Design

### Client (`client.c`)

The client is organized into four functions:

| Function | Purpose |
|---|---|
| `main()` | Orchestrates startup: validates arguments via `validateArguments()`, creates the socket, opens the data file, and enters the send loop. For each line, parses key-value pairs into a cJSON object, serializes it, sends it to the multicast group via UDP, then cleans up. |
| `openFile()` | Prompts the user for a filename and returns an open FILE pointer. Re-prompts on invalid filenames. Uses `rtrim()` to clean input. |
| `parseLine()` | Stateful tokenizer that parses a line of space-separated key:value pairs into a cJSON object. Handles quoted values with escape sequences and unquoted values. Detects value types (boolean, number, string). Returns NULL for empty or invalid lines. |
| `rtrim()` | Strips trailing whitespace (including newline) from a string. Used to clean `fgets()` input. |

### Server (`server.c`)

The server uses two functions — the main loop and a multicast group join helper:

| Function | Purpose |
|---|---|
| `main()` | Validates arguments via `validateArguments()`, creates and binds the socket using `setupSocket()`, joins the multicast group via `joinMulticastGroup()`, then enters an infinite loop calling `recvfrom()`. For each received datagram, parses the JSON string with `cJSON_Parse()`, displays it using `printJSONObject()`, and frees the cJSON object. |
| `joinMulticastGroup()` | Joins the UDP socket to a multicast group. Populates an `ip_mreq` structure with the multicast group address and `INADDR_ANY` for the local interface, then calls `setsockopt()` with `IP_ADD_MEMBERSHIP` to subscribe. Returns 0 on success, -1 on error. |

### Shared Utilities (`utils/utils.c`)

| Function | Purpose |
|---|---|
| `validateArguments()` | Validates command-line arguments shared by both client and server: checks argument count, validates IPv4 address format via `inet_pton()`, verifies the address is in the multicast range (224.0.0.0–239.255.255.255), and validates the port number (numeric, 0–65535). Exits with an error message on any failure. |
| `setupSocket()` | Creates a UDP socket and configures the address structure. In server mode, sets `SO_REUSEADDR` and `SO_REUSEPORT`, binds to `INADDR_ANY`. In client mode, only sets family and port (caller provides IP via `inet_pton()`). |
| `printJSONObject()` | Iterates all children of a cJSON object and prints each key-value pair. Handles strings, booleans, and numbers. Output format varies by mode (client: simple, server: right-aligned columns). |

### Socket Options

- **`SO_REUSEADDR`** and **`SO_REUSEPORT`** allow multiple server processes to bind to the same multicast port, enabling multiple receivers on the same host.
- **`IP_ADD_MEMBERSHIP`** subscribes the server socket to a multicast group so that the OS delivers multicast datagrams addressed to that group.

### JSON Library

This project uses [cJSON](https://github.com/DaveGamble/cJSON), a lightweight JSON parser for C. Key functions used:
- `cJSON_CreateObject()` / `cJSON_Delete()` — Create/free JSON objects
- `cJSON_AddStringToObject()`, `cJSON_AddNumberToObject()`, `cJSON_AddBoolToObject()` — Add typed values
- `cJSON_IsString()`, `cJSON_IsBool()`, `cJSON_IsNumber()` — Check value types
- `cJSON_IsTrue()`, `cJSON_IsFalse()` — Check boolean values
- `cJSON_Print()` — Serialize to formatted JSON string
- `cJSON_PrintUnformatted()` — Serialize to compact JSON string
- `cJSON_Parse()` — Deserialize JSON string to object
- `cJSON_ArrayForEach()` — Iterate object members

## Files

| File | Description |
|---|---|
| `client.c` | UDP multicast client: reads data file, constructs JSON, sends to multicast group |
| `server.c` | UDP multicast server: joins multicast group, receives JSON, deserializes, prints formatted output |
| `cJSON.c` / `cJSON.h` | cJSON library for JSON serialization/deserialization |
| `utils/utils.c` / `utils/utils.h` | Shared socket setup and JSON printing utilities |
| `makefile` | Build configuration |
| `sample.txt` | Provided sample data file for testing |

## Due Date
[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Nqgg2i6Z)
