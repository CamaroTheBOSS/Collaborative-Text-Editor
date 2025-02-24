# Collaborative Text Editor
Server-client architecture console text editor. Built on purpose for learning socket-based network programming. Currently under development.

## Main goal
Main goal of this project is to provide text editor for writing shared documents in real time for Windows without using third-party libraries except winsock api. Most important features
- :white_check_mark: Lightweight, performant text editor with basic functionalities known from other text editors write/erase/select/cut-copy-paste/find-replace/undo-redo
- :white_check_mark: Responsive UI
- :white_check_mark: Up to 8 connected clients per document and up to ~32k lines long documents
- :white_check_mark:Low CPU and memory, thread pool based server where load is balanced evenly (based on number of documents served by the thread) with defined, finite number of threads
- :white_check_mark: Own communication protocol based on TCP/IP
- :white_check_mark: AI free

## Prerequisities
- Windows
- Ws2_32.dll
