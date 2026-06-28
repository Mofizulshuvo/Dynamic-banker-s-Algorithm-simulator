#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <chrono>

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
    RESUME_PROCESS,
    TERMINATE_PROCESS,
    MANUAL_REALLOCATION
};

// Fault Types
enum FaultType
{
    RESOURCE_LOSS,
    MEMORY_FRAGMENTATION,
    HARDWARE_FAILURE
};

// Process
struct Process
{
    int id;
    ProcessStatus status;
    int progress; // 0-100 percentage
};

// Fault Event
struct FaultEvent
{
    int id;
    FaultType type;
    int resourceID;
    int unitsLost;
    string description;
    long long timestamp;
};

// Recovery Action
struct RecoveryAction
{
    int id;
    RecoveryType type;
    int processID;
    int resourceID;
    int units;
    bool success;
    string message;
    long long timestamp;
};

// Timeline Event
struct TimelineEvent
{
    int id;
    string event;
    int processID;
    vector<int> available;
    bool safe;
    long long timestamp;
};

// Execution Step
struct ExecutionStep
{
    int stepNumber;
    int processID;
    string action;
    vector<int> available;
    bool safe;
    string message;
};

// Complete System State
struct SystemState
{
    int processCount;
    int resourceCount;

    vector<vector<int>> allocation;
    vector<vector<int>> maximum;
    vector<vector<int>> need;

    vector<int> available;
    vector<int> totalResources; // Total system resources

    vector<int> safeSequence;

    vector<Process> processes;
    vector<FaultEvent> faultHistory;
    vector<RecoveryAction> recoveryHistory;
    vector<TimelineEvent> timeline;

    SimulationState simulationState;
    int currentStep;
    bool simulationFinished;
    int simulationSpeed; // milliseconds per step
};

#endif