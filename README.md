# Collaborative Text Editor
Server-client architecture console text editor. Built on purpose for learning socket-based network programming.

## Main goal
Main goal of this project is to provide text editor for writing shared documents in real time for Windows without using third-party libraries except winsock api. Most important features
- :white_check_mark: Lightweight text editor with basic functionalities known from other text editors write/erase/select/cut-copy-paste/find-replace/undo-redo
- :white_check_mark: File-based database for containing users data and documents data
- :white_check_mark: Responsive UI
- :white_check_mark: Up to 8 connected clients per document and up to ~32k lines long documents
- :white_check_mark:Low CPU and memory, thread pool based server where load is balanced evenly (based on number of documents served by the thread) with defined, finite number of threads
- :white_check_mark: Own communication protocol based on TCP/IP
- :white_check_mark: AI free code

## Description
Project makes possible editing file from multiple clients. Connection to existing session is possible via access code which is created after the session is created (load/create document). Basic authentication is provided via login and password.

## Prerequisities
- Windows
- Ws2_32.dll
- C++20

## Components
- Server for controlling state of documents between multiple clients
- Client is simple interface where users can login/create/load/join/delete docs and collaborate with other clients around the world
- RandomClient is a client created for testing purposes - it performs random actions
- No build script provided. Please build with Visual Studio MSVC

## How to run
- build Server, Client and RandomClient
To run server:
```
Server.exe --ip <server_ip> --port <server_port>
```
To run client:
```
Client.exe run --ip <server_ip> --port <server_port>
```
To run random client:
```
RandomClient.exe join --ip <server_ip> --port <server_port> --login <login> --password <password> --access-code <access_code>
```
For more information run
```
<binary> help
```

## Scripts
- client.py for creating multiple sessions and connecting multiple random clients into specific session
