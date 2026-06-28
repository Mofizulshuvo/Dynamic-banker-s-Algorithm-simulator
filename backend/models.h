#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>

using namespace std;

// Process Status
enum ProcessStatus
{
    WAITING,
    RUNNING,
    FINISHED,
    SUSPENDED,
    TERMINATED
};

// Simulation State
enum SimulationState
{
    IDLE,
    INITIALIZED,
    SAFE,
    UNSAFE,
    RUNNING_STATE,
    PAUSED,
    FAULT,
    RECOVERY,
    COMPLETED
};

// Recovery Types
enum RecoveryType
{
    RESTORE_RESOURCE,
    SUSPEND_PROCESS,
    WAIT_FOR_RESOURCE,
    TERMINATE_PROCESS,
    MANUAL_REALLOCATION
};

// Process
struct Process
{
    int id;

    ProcessStatus status;
};


// Fault Event
struct FaultEvent
{
    int resourceID;

    int unitsLost;

    string description;
};

// Recovery Action
struct RecoveryAction
{
    RecoveryType type;

    int processID;

    bool success;

    string message;
};


// Execution Step
// Used for animation & timeline
struct ExecutionStep
{
    int stepNumber;

    int processID;

    string action;

    vector<int> available;

    bool safe;

    string message;
};

#endif