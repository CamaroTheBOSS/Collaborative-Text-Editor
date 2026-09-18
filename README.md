# Collaborative Text Editor
Server-client architecture console text editor. Built on purpose for learning socket-based network programming.

## Project
Text editor for writing shared documents in real time for Windows without using third-party libraries. Most important features
- :white_check_mark: Basic text editor functionality write/erase/select/cut-copy-paste/find-replace/undo-redo
- :white_check_mark: File-based database for containing users data and documents data
- :white_check_mark: In-console UI
- :white_check_mark: Up to 8 connected clients per document and up to ~32k lines long documents
- :white_check_mark: Thread pool based server where load is balanced evenly (based on number of documents served by the thread) with defined, finite number of threads
- :white_check_mark: Custom communication protool built on top of TCP/IP
- :white_check_mark: AI free code

## Description
Project makes possible editing file from multiple clients. Connection to existing session is possible via access code which is created after the session is created (load/create document). Basic authentication is provided via login and password.

## Prerequisities
- Windows
- Windows SDK

## Components
- ServerMain for controlling state of documents between multiple clients
- ClientMain is simple interface where users can login/create/load/join/delete docs and collaborate with other clients around the world
- RandomClient is a client created for testing purposes - it performs random actions
- Test project is for unit tests
- TestRandomClientServerSync is e2e client-server synchronization tester

## Build
Open CollaborativeTextEditor.sln with Visual Studio. Build whole solution to build executables with tests or just \[BUILD\] project for building only ServerMain, ClientMain and RandomClient.

## Run

From repository root directory:

Go to the output directory (<configuration> => (Debug | Release))
```
cd ./x64/<configuration>/
```

Run the server:
```
./ServerMain.exe --log-level debug
```

Run the client:
```
./ClientMain.exe
```

Register user, log in and create new document. Grab the session access code and run another ClientMain or RandomClient for joining the collaborative session.

To run the random client firstly pre register the user from your ClientMain and then use this user to start the random client:
```
./RandomClient --login <user> --password <pass> --access-code <code>
```

For more information run for guidance:
```
<binary> help
```

## Scripts
- client.py for creating multiple sessions and connecting multiple random clients into specific session

  To run the script:
  - Run manually the first ClientMain
  - Pre-register all the users you want to use in client.py
  - Create a doc and connect to it
  - Run client.py
    ```
    cd ./scripts
    python -m client --login <login1>,<login2>,<login3>... --password <pass1>,<pass2>,<pass3>... --access-code <code> --exe-path <path_to_ClientMain_or_RandomClient>
    ```

    Run `python -m client --help` for guidance
