# Kontroller

Kontroller is a simple library to read input from and control the LEDs of the KORG nanoKONTROL2. Communication can either be done directly (via MIDI), or using a client / server (where the server communicates via MIDI and broadcasts messages using TCP sockets to any connected clients). The client / server allow MIDI state to be maintained between runs of an application (as long as the server is kept running in the background, e.g. using the provided Windows service), and allow multiple applications (or even multiple computers) to read MIDI state concurrently.

## Building

### Platforms

Windows and macOS are fully supported. Linux can read from the MIDI controller, but not write to it (control the LEDs).

### Build System

Kontroller uses CMake as a meta build system. Run CMake (with the provided Build directory) to generate project files for your compiler / IDE of choice.

### Options

The following CMake options are provided:

* `KONTROLLER_BUILD_SERVICE` - Whether to generate the service project (Windows only)
* `KONTROLLER_BUILD_EXAMPLES` - Whether to generate the example projects

### Targets

There are two main build targets:

1. `Kontroller` - The Kontroller library (including direct MIDI communication, and a client / server)
2. `KontrollerService` - A Windows service that runs a Kontroller server

Plus, a handful of example targets:

* `KontrollerExample-Device` - Demonstrates how to read values from and control the LEDs of a nanoKONTROL2 via MIDI
* `KontrollerExample-Pong` - Pong, played on the nanoKONTROL2 (ball displayed using the LEDs, paddles are the first and last sliders)
* `KontrollerExample-Server` - Demonstrates how to host a socket server, which broadcasts messages to any connected clients
* `KontrollerExample-Client` - Demonstrates how to run a client, which receives messages from a server

### Dependencies

Kontroller comes will all necessary dependencies as git submodules, which can be obtained via calls to `git submodule init` and `git submodule update`.

## Usage

### Direct / MIDI

Create a `Kontroller::Device` to interact with a nanoKONTROL2 over MIDI. The `Device` object creates a thread to communicate with the MIDI controller, and automatically attempts to reconnect if the connection is lost. You can check the connection status by calling `isConnected()`.

The current state of the MIDI controller can be polled by calling `getState()`, and callback functions are provided to send notifications when values change (callbacks are fired from the created thread).

LEDs can be controlled by first calling `enableLEDControl()`, then calling `setLEDOn()`.

### Client / Server

Crerate a `Kontroller::Server` to start the socket server. The server will manage a `Kontroller::Device` and host a TCP listen socket, and will automatically retry if creation of the listen socket fails. When a client connects, the server sends along the current total state. When any state changes, the server sends the updates to all connected clients. You can check the status of the server by calling `isListening()`.

Create a `Kontroller::Client` to start a socket client. The client will attempt to connect to a server at the provided address, and will automatically retry if the connection fails. You can check the connection status by calling `isConnected()`. Similar to the `Kontroller::Device`, the current state can be queried by calling `getState()`, and callback functions are available (which fire on a separate thread).
