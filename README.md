# Lab 2 - JSON Object Transmission

- **Author:** Elijah Heimsoth
- **Course:** Network Programming 3762
- **Date:** 02-03-2026

## Overview

This lab implements a UDP client-server system for transmitting serialized JSON objects. The client reads key-value pair data from a user-specified file, constructs JSON objects using the cJSON library, serializes them to JSON strings, and sends them to a server over UDP. The server receives the serialized JSON strings, deserializes them back into JSON objects, and displays each key-value pair in a formatted, human-readable output.

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
./server <port>
```

Example:

```bash
./server 5000
```

- `port` must be between 0 and 65535

The server runs continuously until interrupted with Ctrl+C.

### 2. Start the client

```bash
./client <ipaddr> <port>
```

Example:

```bash
./client 127.0.0.1 5000
```

The client will prompt for the name of a message file (e.g., `sample.txt`). It reads the file line by line, parses each line into a JSON object, and sends the serialized JSON to the server.

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
| `main()` | Orchestrates startup: validates arguments (IP address and port), creates the socket, opens the data file, and enters the send loop. For each line, parses key-value pairs into a cJSON object, serializes it, sends it via UDP, then cleans up. |
| `openFile()` | Prompts the user for a filename and returns an open FILE pointer. Re-prompts on invalid filenames. Uses `rtrim()` to clean input. |
| `parseLine()` | Stateful tokenizer that parses a line of space-separated key:value pairs into a cJSON object. Handles quoted values with escape sequences and unquoted values. Detects value types (boolean, number, string). Returns NULL for empty or invalid lines. |
| `rtrim()` | Strips trailing whitespace (including newline) from a string. Used to clean `fgets()` input. |

### Server (`server.c`)

The server uses a simple main function with an infinite receive loop:

| Function | Purpose |
|---|---|
| `main()` | Validates arguments (port number), creates and binds the socket using `setupSocket()`, then enters an infinite loop calling `recvfrom()`. For each received datagram, parses the JSON string with `cJSON_Parse()`, displays it using `printJSONObject()`, and frees the cJSON object. |

### Shared Utilities (`utils/utils.c`)

| Function | Purpose |
|---|---|
| `setupSocket()` | Creates a UDP socket and configures the address structure. In server mode, sets `SO_REUSEADDR` and `SO_REUSEPORT`, binds to `INADDR_ANY`. In client mode, only sets family and port (caller provides IP via `inet_pton()`). |
| `printJSONObject()` | Iterates all children of a cJSON object and prints each key-value pair. Handles strings, booleans, and numbers. Output format varies by mode (client: simple, server: right-aligned columns). |

### Socket Options

- **`SO_REUSEADDR`** and **`SO_REUSEPORT`** allow multiple server processes to bind to the same port.

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
| `client.c` | UDP client: reads data file, constructs JSON, sends to server |
| `server.c` | UDP server: receives JSON, deserializes, prints formatted output |
| `cJSON.c` / `cJSON.h` | cJSON library for JSON serialization/deserialization |
| `utils/utils.c` / `utils/utils.h` | Shared socket setup and JSON printing utilities |
| `makefile` | Build configuration |
| `sample.txt` | Provided sample data file for testing |

## Due Date
[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/Nqgg2i6Z)
