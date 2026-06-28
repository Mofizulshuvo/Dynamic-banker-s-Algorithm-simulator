# Dynamic Banker's Algorithm Simulator

A comprehensive, interactive simulator for the Banker's Algorithm with runtime fault injection and recovery mechanisms. This project demonstrates OS resource management, deadlock avoidance, and system recovery strategies.

## Features

- **Interactive Web Interface**: Modern, responsive UI with dark mode support
- **Runtime Simulation**: Step-by-step execution with timeline visualization
- **Fault Injection**: Inject various fault types during simulation (resource loss, memory fragmentation, hardware failure)
- **Recovery Strategies**: Multiple recovery options (restore resource, suspend/resume/terminate process, manual reallocation)
- **REST API Backend**: C++17 backend with HTTP server using cpp-httplib
- **Real-time Updates**: Live status updates for processes, resources, and system state
- **Process Cards**: Visual representation of process states and progress
- **Resource Bars**: Animated resource utilization indicators
- **Timeline Logging**: Complete event history with timestamps

## Architecture

### Backend (C++17)
- **models.h**: Data structures for processes, resources, faults, recovery actions, timeline events
- **banker.h/cpp**: Banker's Algorithm implementation with resource request validation
- **fault.h/cpp**: Fault injection engine with multiple fault types
- **recovery.h/cpp**: Recovery strategies with history tracking
- **simulation.h/cpp**: Simulation orchestration with state management
- ** server.h/cpp**: HTTP server with REST API endpoints
- **main.cpp**: Server entry point

### Frontend (HTML5, CSS3, Vanilla JavaScript)
- **index.html**: Single-page application structure
- **style.css**: Modern styling with glassmorphism and animations
- **script.js**: API integration and UI logic

## Dependencies

### Backend
- **OS**: Windows 10 or later (required by cpp-httplib)
- **Compiler**: C++17 compiler (g++ or MSVC)
- cpp-httplib (single-header library included in `backend/external/`)
- nlohmann/json (single-header library included in `backend/external/`)
- Windows sockets library (ws2_32) for networking

### Frontend
- Modern web browser (Chrome, Firefox, Edge, Safari)
- No external dependencies required

## Installation

1. Clone or download the repository
2. Ensure g++ is installed and in your PATH
3. No additional installation required for external libraries (included)

## Usage

### Quick Start

1. Run `run.bat` on Windows
2. The script will:
   - Compile the C++ backend
   - Start the HTTP server on port 8080
   - Open the web interface in your browser
3. Use the web interface to:
   - Edit the allocation and max matrices
   - Click "Initialize" to set up the system
   - Click "Run" to start the simulation
   - Use "Inject Fault" to test fault scenarios
   - Use "Recovery" to apply recovery strategies

### Manual Compilation

```bash
g++ -std=c++17 backend/main.cpp backend/server.cpp backend/simulation.cpp backend/banker.cpp backend/fault.cpp backend/recovery.cpp -o backend/server.exe -lws2_32
```

### Running the Server

```bash
backend\server.exe
```

The server will start on `http://localhost:8080` and serve both the API and the frontend.

## API Endpoints

### System Status
- `GET /api/status` - Get current system state

### Simulation Control
- `POST /api/initialize` - Initialize the simulation with given matrices
- `POST /api/run` - Start the simulation
- `POST /api/step` - Execute one simulation step
- `POST /api/pause` - Pause the simulation
- `POST /api/resume` - Resume the simulation
- `POST /api/reset` - Reset the simulation

### Fault Injection
- `POST /api/fault` - Inject a fault
  - Body: `{ type, resourceID, unitsLost }`
  - Types: `RESOURCE_LOSS`, `MEMORY_FRAGMENTATION`, `HARDWARE_FAILURE`

### Recovery
- `POST /api/recover` - Apply a recovery strategy
  - Body: `{ type, processID, resourceID, units, fromProcess, toProcess }`
  - Types: `RESTORE_RESOURCE`, `SUSPEND_PROCESS`, `RESUME_PROCESS`, `TERMINATE_PROCESS`, `MANUAL_REALLOCATION`

### Speed Control
- `POST /api/speed` - Set simulation speed
  - Body: `{ speed }` (in milliseconds)

## Banker's Algorithm

The simulator implements the classic Banker's Algorithm for deadlock avoidance:

1. **Need Matrix**: Calculated as `Need = Max - Allocation`
2. **Safety Check**: Determines if a safe sequence exists
3. **Resource Request**: Validates requests before granting
4. **Safe Sequence**: Order in which processes can complete

## Fault Types

- **Resource Loss**: Sudden reduction in available resources
- **Memory Fragmentation**: Resources become unusable due to fragmentation
- **Hardware Failure**: Complete failure of a resource type

## Recovery Strategies

- **Restore Resource**: Add resources back to the available pool
- **Suspend Process**: Temporarily pause a process to free its resources
- **Resume Process**: Reactivate a suspended process
- **Terminate Process**: Permanently stop a process and release all resources
- **Manual Reallocation**: Manually redistribute resources between processes

## Simulation States

- `IDLE`: Not initialized
- `INITIALIZED`: Ready to run
- `SAFE`: System is in safe state
- `UNSAFE`: System is in unsafe state (deadlock risk)
- `RUNNING`: Simulation is executing
- `PAUSED`: Simulation is paused
- `FAULT`: Fault has been detected
- `RECOVERY`: Recovery is in progress
- `COMPLETED`: Simulation has finished

## Project Structure

```
Dynamic-banker's Algorithm-simulator/
├── backend/
│   ├── external/
│   │   ├── httplib.h          # cpp-httplib library
│   │   └── json.hpp           # nlohmann/json library
│   ├── models.h               # Data structures
│   ├── banker.h/cpp           # Banker's Algorithm
│   ├── fault.h/cpp            # Fault injection
│   ├── recovery.h/cpp         # Recovery strategies
│   ├── simulation.h/cpp       # Simulation orchestration
│   ├── server.h/cpp           # HTTP server
│   └── main.cpp               # Entry point
├── frontend/
│   ├── index.html             # Web interface
│   ├── style.css              # Styling
│   └── script.js              # Frontend logic
├── run.bat                    # Build and run script
└── README.md                  # This file
```

## Default Configuration

The simulator starts with a classic example:
- 5 Processes (P0-P4)
- 3 Resources (A, B, C)
- Total Resources: [10, 5, 7]
- Available: [3, 3, 2]

## Troubleshooting

### Compilation Errors
- Ensure g++ supports C++17 (`g++ --version`)
- Verify Windows sockets library is available
- Check that all source files exist

### Server Won't Start
- Check if port 8080 is already in use
- Verify firewall allows the application
- Check server console for error messages

### Frontend Not Connecting
- Ensure the server is running
- Check browser console for API errors
- Verify CORS is enabled (default)

## License

This project is for educational purposes.

## Contributing

This is an educational project. Feel free to fork and modify for learning purposes.

## Acknowledgments

- Banker's Algorithm by Edsger Dijkstra
- cpp-httplib library by yhirose
- nlohmann/json library by nlohmann
