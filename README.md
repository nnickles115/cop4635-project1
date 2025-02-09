# COP4635 Project 1 - HTTP Server

## Description
**Author:** Noah Nickles<br>
**Course:** COP4635 - Sys & Net II<br>

## Features
 - **HTTP/1.1 Support:** Implements core functionality for handling HTTP/1.1 requests and responses.
 - **GET Request Handling:** Serves static and dynamic in response to `GET` requests.
 - **POST Request Handling:** Supports processing URL-encoded `POST` requests, allowing for basic form submissions.
 - **Customizable Server Configuration:**
    - **Port Number:** Specify the listening port using the `-p` or `--port` argument.
    - **Root Directory:** Configure the web content root directory using the `-r` or `--root` argument.
    - **Index File:** Set a custom index file to be served at the root URL using the `-i` or `--index` argument.
    - **Thread Count:** Optionally enable multi-threading and control the number of worker threads using the `-t` or `--threads` argument.
- **Optional Multi-threading:**  Leverages a thread pool to serve content more efficiently and handle concurrent requests. Can be disabled to run in single-threaded mode.
- **Logging:** Provides logging output to the console, with configurable verbosity levels (DEBUG, INFO, WARN, ERROR) controlled by command-line arguments.
- **Error Handling:** Implements basic error handling, including returning 404 Not Found responses for missing files and handling invalid command-line arguments with informative error messages.

## Installation
### Prerequisites
Before you can compile and run the HTTP server, you will need to have the following prerequisites installed on your system:

 - **C++ Compiler:** A C++17 compatible compiler is required. Recommended: **g++ (GNU Compiler Collection) version 8 or later** or a compatible compiler that fully supports the C++17 standard.
 - **Make Build System:** The `make` build system is used to automate the compilation process.

### Steps
 1. Clone this repo using:
 ```bash
 git clone https://github.com/nnickles115/cop4635-project1.git
 ```
 2. Open a terminal in the directory where the repo was cloned and type:
 ```bash
 make
 ```
 3. Refer to the usage steps below.

## Usage
 - After building, simply type: `./server` to start the server.
 - The server will output what URL it is running on.
 - Open any web browser and either type in the URL, or Ctrl + Left Click the URL in the terminal window to view the index page.

## Optional Arguments:
 - `-p <port_number>` or `--port <port_number>`: Specifies the port number for the server to listen on. Replace the `<port_number>` with the desired port number.
 
 **Example:** To start the server on port 8080, use:
 ```bash
 ./server -p 8080
 ```
****
 - `-r <file_path>` or `--root <file_path>`: Specifies the location of the web content to be served. Replace the `<file_path>` with either an absolute file path, or a relative path to the executable.

 **Example:** To choose a root directory in /srv/www, use:
 ```bash
 ./server -r /srv/www
 ```
****
 - `-i <file.ext>` or `--index <file.ext>`: Specifies the index file that is hosted at the root directory `/`. Replace the `<file.ext>` with the name of the file AND the extension.

 **Example:** To choose a specific file inside of the root directory, use:
 ```bash
 ./server -i myFile.html
 ```
****
 - `-t <number>` or `--threads <number>`: Specifies the number of threads to use. Replace the `<number>` with a number greater than or equal to `0`. To disable multi-threading specify `0`.

 **Example:** To use `8` threads, use:
 ```bash
 ./server -t 8
 ```
****
 **Other arguments:**
 - `-d` or `--debug` enables `DEBUG` messages along with normal output.

 **Example:** To enable debug messages, use: 
 ```bash
 ./server -d
 ```
 The default logging level is `INFO`
****
**Default server config:** If no args are specified, these defaults will be used.
- `port:` 60001
- `debug:` false
- `root:` ./www
- `indexFile:` index.html
- `threadCount:` 4