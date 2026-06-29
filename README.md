# Dynamic Banker's Algorithm Simulator with Resource Fragmentation and Fault Modeling

A portfolio-quality Operating Systems project that demonstrates deadlock avoidance with Banker's Algorithm, runtime resource faults, and recovery strategies through an interactive web dashboard.

## Project Overview

This simulator models how an operating system manages resources while processes execute. It calculates the Need matrix, evaluates safe sequences, runs processes step by step, injects runtime failures, and applies recovery mechanisms when the system becomes unsafe.

The backend is written in C++17 and exposes a REST API with `cpp-httplib` and `nlohmann/json`. The frontend is a vanilla HTML, CSS, and JavaScript application served by the backend.

## Problem Statement

Traditional Banker's Algorithm examples usually stop at checking whether a static allocation state is safe. Real systems are dynamic: processes execute, resources are released, hardware can fail, memory can fragment, and the OS must recover without entering deadlock.

This project extends the classic algorithm into a dynamic simulation where faults can occur during execution and recovery decisions are visualized in real time.

## Objectives

- Demonstrate Safe State and Unsafe State clearly.
- Visualize Allocation, Maximum, Need, Available, and Total resources.
- Animate process execution and resource release.
- Inject faults only during runtime.
- Recalculate Banker's Algorithm after faults and recovery.
- Provide recovery methods for unsafe states.
- Make deadlock examples easy to test and explain.
- Provide a professional dashboard suitable for a university presentation or portfolio.

## Features

- Editable process/resource configuration.
- Automatic Need matrix calculation.
- Safe sequence calculation.
- Run, Pause, Resume, Next Step, Reset, and Speed controls.
- Runtime fault injection:
  - Resource Loss
  - Memory Fragmentation
  - Hardware Failure
- Recovery strategies:
  - Restore Lost Resource
  - Suspend Process
  - Resume Process
  - Terminate Process
  - Manual Resource Reallocation
- Live resource bars.
- Process cards with status, allocation, need, and progress.
- Runtime timeline for every major event.
- Configuration validation with user-friendly messages.
- Safe sample and deadlock preset.
- Dark mode.
- Responsive modern dashboard UI.

## Technology Stack

Backend:

- C++17
- Object-Oriented Programming
- `cpp-httplib`
- `nlohmann/json`
- Windows sockets library through MinGW-w64

Frontend:

- HTML5
- CSS3
- Vanilla JavaScript

No React, Vue, Angular, Bootstrap, Electron, Qt, Java, PHP, Python, or Node backend is used.

## Architecture

```text
Frontend UI
    |
    | REST API
    v
cpp-httplib C++ Server
    |
    +-- Simulation
    |     Coordinates runtime state, steps, faults, recovery, and timeline
    |
    +-- Banker
    |     Calculates Need matrix and safe sequence
    |
    +-- FaultEngine
    |     Applies resource loss, fragmentation, and hardware failure
    |
    +-- RecoveryEngine
          Restores resources, suspends/resumes/terminates processes,
          and manually reallocates resources
```

## Folder Structure

```text
Dynamic-banker's Algorithm-simulator/
|-- backend/
|   |-- main.cpp
|   |-- server.cpp
|   |-- server.h
|   |-- simulation.cpp
|   |-- simulation.h
|   |-- banker.cpp
|   |-- banker.h
|   |-- fault.cpp
|   |-- fault.h
|   |-- recovery.cpp
|   |-- recovery.h
|   |-- models.h
|   |-- external/
|       |-- httplib.h
|       |-- json.hpp
|-- frontend/
|   |-- index.html
|   |-- style.css
|   |-- script.js
|-- run.bat
|-- README.md
```

## Algorithms

### Need Matrix

```text
Need[i][j] = Maximum[i][j] - Allocation[i][j]
```

### Safety Algorithm

1. Copy Available into Work.
2. Mark all processes as unfinished.
3. Find a process whose Need is less than or equal to Work.
4. Pretend that process finishes and releases its allocation.
5. Repeat until no more processes can finish.
6. If every active process can finish, the system is safe.
7. Otherwise, the system is unsafe.

### Fault Handling

Faults reduce available resources during execution. After every fault:

1. The timeline records the event.
2. Need is recalculated.
3. Safe sequence is recalculated.
4. If no safe sequence exists, the system enters `UNSAFE`.
5. Recovery actions become necessary.

### Recovery Handling

Every recovery strategy immediately reruns the safety algorithm. If the system becomes safe again, execution can continue.

## API Endpoints

All responses are JSON.

```text
GET  /api/health
GET  /api/status
POST /api/initialize
POST /api/run
POST /api/step
POST /api/pause
POST /api/resume
POST /api/fault
POST /api/recover
POST /api/reset
POST /api/speed
```

Short aliases without `/api` are also available for the main endpoints.

## How to Build and Run

Open PowerShell in the project root:

```powershell
cd "C:\Users\Mofiz\OneDrive\Desktop\SHUVO\OS project\Dynamic-banker's Algorithm-simulator"
.\run.bat
```

The script will:

1. Stop an old `server.exe` if it is locking the build.
2. Compile the backend.
3. Start the HTTP server.
4. Open the frontend.

Then visit:

```text
http://localhost:8080
```

## Manual Build

```powershell
g++ -std=c++17 -D_WIN32_WINNT=0x0A00 -DWIN32_LEAN_AND_MEAN -DNOMINMAX backend/main.cpp backend/server.cpp backend/simulation.cpp backend/banker.cpp backend/fault.cpp backend/recovery.cpp -o backend/server.exe -lws2_32 -static
.\backend\server.exe
```

Open:

```text
http://localhost:8080
```

Keep the server window open while using the simulator.

## Demo Scenarios

### Safe Sample

Click `Load Sample`, then:

1. `Initialize`
2. `Run`

Expected safe sequence:

```text
P1 -> P3 -> P4 -> P0 -> P2
```

### Deadlock / Unsafe Sample

Click `Load Deadlock`, then:

1. `Initialize`
2. `Run`

Expected result:

```text
System State: UNSAFE
Safe Sequence: None
```

This example represents:

```text
P0 holds A and waits for B
P1 holds B and waits for A
```

## Screenshots

Add screenshots here after running the application:

```text
assets/screenshots/dashboard.png
assets/screenshots/deadlock-result.png
assets/screenshots/fault-recovery.png
```

## Future Improvements

- Export timeline as a report.
- Add multiple built-in OS case studies.
- Add chart-based resource history.
- Add automated presentation mode.
- Add persistence for saved configurations.

## Contributors

- Mofiz

## License

This project is for educational use.
