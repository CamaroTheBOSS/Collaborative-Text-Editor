# Collaborative Text Editor
Server-client archituctere console text editor. Built on purpose for learning socket-based network programming. Currently under development

## Main goal
Main goal of this project is to provide text editor for writing shared documents in real time for Windows without using third-party libraries except winsock api. Most important features
- Lightweight, performant text editor with basic functionalities known from other text editors write/erase/select/cut-copy-paste/find-replace/undo-redo
- Responsive UI
- Up to 8 connected clients per document and up to ~32k lines long documents
- Low CPU and memory, thread pool based server where load is balanced evenly (based on number of documents served by the thread) with defined, finite number of threads
- Own communication protocol based on TCP/IP

## Prerequisities
- Windows
- Ws2_32.dll
