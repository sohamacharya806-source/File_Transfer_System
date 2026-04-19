# File_Transfer_System# File Transfer System — Setup Guide

This project is a lightweight *client-server file transfer system in C* built over TCP/IP sockets. It allows sending any file (PDF, TXT, images, etc.) between machines using a simple and reliable protocol.

---

## Prerequisites

Make sure you have the following installed:

* GCC (MinGW for Windows / system GCC for Linux/macOS)
* Git
* Basic knowledge of terminal/command prompt
* OS:

  * Windows (uses Winsock2)
  * Linux/macOS (POSIX sockets)

---

## Clone the Repository

bash
git clone <your-repo-url>
cd File_Transfer_System


---

## Project Structure


File_Transfer_System/
├── src/
│   ├── server.c
│   └── client.c
├── include/
│   └── common.h
├── frontend/
│   └── index.html
├── build.bat
├── server.exe
├── client.exe
├── test.txt
└── README.md


---

## Build the Project

### Windows

Run the build script:

bat
build.bat


Or manually:

bat
gcc src/server.c -o server.exe -lws2_32
gcc src/client.c -o client.exe -lws2_32


---

### Linux / macOS

bash
gcc src/server.c -o server
gcc src/client.c -o client


---

## Run the Application

### Step 1 — Start the Server

bash
./server.exe


Output:


[server] Listening on port 8080...


---

### Step 2 — Run the Client

bash
./client.exe <server_ip> <file_path>


#### Examples:

bash
./client.exe 127.0.0.1 test.txt
./client.exe 192.168.1.42 report.pdf


---

### Step 3 — Stop the Server

Type:


exit


---

## How It Works

The system uses a simple TCP protocol:

1. Client sends filename length (4 bytes)
2. Client sends filename
3. Client sends file data

Server reads data until connection closes and saves the file.

---

## Features

* TCP-based reliable transfer
* Supports all file types
* Cross-platform (Windows + Linux/macOS)
* Persistent server (handles multiple clients sequentially)
* Path traversal protection
* Graceful shutdown support

---

## Configuration

Edit include/common.h:

c
#define PORT        8080
#define BUFFER_SIZE 1024


Recompile after making changes.

---

## Quick Test

Run on same machine:

bash
# Terminal 1
./server.exe

# Terminal 2
./client.exe 127.0.0.1 test.txt


---

## Security Notes

* No encryption or authentication
* Intended for local/educational use only
* Do NOT expose to public internet

---

## Important Notes

* File is saved in the directory where server runs
* Do NOT modify shared files without coordination
* Pull latest changes before working:

bash
git pull origin <branch-name>


---

## Authors

ICS Project — File Transfer System

---

## License

For educational use. Modify freely.