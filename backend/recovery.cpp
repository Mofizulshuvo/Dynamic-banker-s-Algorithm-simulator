#include "recovery.h"

#include <iostream>

using namespace std;

// Constructor
RecoveryEngine::RecoveryEngine()
{
    recoveryCounter = 0;
}

// Get Current Timestamp
long long RecoveryEngine::getCurrentTimestamp()
{
    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(duration).count();
}

// Restore Lost Resource
RecoveryAction RecoveryEngine::restoreResource(
    SystemState &state,
    int resourceID,
    int units
)
{
    RecoveryAction action;
    action.id = recoveryCounter++;
    action.type = RESTORE_RESOURCE;
    action.resourceID = resourceID;
    action.units = units;
    action.timestamp = getCurrentTimestamp();

    if (resourceID < 0 || resourceID >= state.resourceCount)
    {
        action.success = false;
        action.message = "Invalid resource ID";
        return action;
    }

    if (units <= 0)
    {
        action.success = false;
        action.message = "Invalid units to restore";
        return action;
    }

    state.available[resourceID] += units;
    action.success = true;
    action.message = "Restored " + to_string(units) + " units of resource " + to_string(resourceID);

    state.recoveryHistory.push_back(action);
    return action;
}

// Suspend Process
RecoveryAction RecoveryEngine::suspendProcess(
    SystemState &state,
    int processID
)
{
    RecoveryAction action;
    action.id = recoveryCounter++;
    action.type = SUSPEND_PROCESS;
    action.processID = processID;
    action.timestamp = getCurrentTimestamp();

    if (processID < 0 || processID >= state.processCount)
    {
        action.success = false;
        action.message = "Invalid process ID";
        return action;
    }

    state.processes[processID].status = SUSPENDED;
    action.success = true;
    action.message = "Suspended process P" + to_string(processID);

    state.recoveryHistory.push_back(action);
    return action;
}

// Resume Process
RecoveryAction RecoveryEngine::resumeProcess(
    SystemState &state,
    int processID
)
{
    RecoveryAction action;
    action.id = recoveryCounter++;
    action.type = RESUME_PROCESS;
    action.processID = processID;
    action.timestamp = getCurrentTimestamp();

    if (processID < 0 || processID >= state.processCount)
    {
        action.success = false;
        action.message = "Invalid process ID";
        return action;
    }

    state.processes[processID].status = WAITING;
    action.success = true;
    action.message = "Resumed process P" + to_string(processID);

    state.recoveryHistory.push_back(action);
    return action;
}

// Terminate Process
RecoveryAction RecoveryEngine::terminateProcess(
    SystemState &state,
    int processID
)
{
    RecoveryAction action;
    action.id = recoveryCounter++;
    action.type = TERMINATE_PROCESS;
    action.processID = processID;
    action.timestamp = getCurrentTimestamp();

    if (processID < 0 || processID >= state.processCount)
    {
        action.success = false;
        action.message = "Invalid process ID";
        return action;
    }

    state.processes[processID].status = TERMINATED;

    // Release allocated resources
    for (int i = 0; i < state.resourceCount; i++)
    {
        state.available[i] += state.allocation[processID][i];
        state.allocation[processID][i] = 0;
        state.need[processID][i] = 0;
    }

    action.success = true;
    action.message = "Terminated process P" + to_string(processID) + " and released all resources";

    state.recoveryHistory.push_back(action);
    return action;
}

// Manual Resource Reallocation
RecoveryAction RecoveryEngine::manualReallocation(
    SystemState &state,
    int fromProcess,
    int toProcess,
    int resourceID,
    int units
)
{
    RecoveryAction action;
    action.id = recoveryCounter++;
    action.type = MANUAL_REALLOCATION;
    action.processID = fromProcess;
    action.resourceID = resourceID;
    action.units = units;
    action.timestamp = getCurrentTimestamp();

    if (fromProcess < 0 || fromProcess >= state.processCount)
    {
        action.success = false;
        action.message = "Invalid source process ID";
        return action;
    }

    if (toProcess < 0 || toProcess >= state.processCount)
    {
        action.success = false;
        action.message = "Invalid target process ID";
        return action;
    }

    if (resourceID < 0 || resourceID >= state.resourceCount)
    {
        action.success = false;
        action.message = "Invalid resource ID";
        return action;
    }

    if (units <= 0)
    {
        action.success = false;
        action.message = "Invalid units to reallocate";
        return action;
    }

    if (state.allocation[fromProcess][resourceID] < units)
    {
        action.success = false;
        action.message = "Source process doesn't have enough allocated resources";
        return action;
    }

    state.allocation[fromProcess][resourceID] -= units;
    state.allocation[toProcess][resourceID] += units;

    // Update need matrices
    state.need[fromProcess][resourceID] = state.maximum[fromProcess][resourceID] - state.allocation[fromProcess][resourceID];
    state.need[toProcess][resourceID] = state.maximum[toProcess][resourceID] - state.allocation[toProcess][resourceID];

    action.success = true;
    action.message = "Reallocated " + to_string(units) + " units of resource " + to_string(resourceID) + " from P" + to_string(fromProcess) + " to P" + to_string(toProcess);

    state.recoveryHistory.push_back(action);
    return action;
}

// Get Recovery History
vector<RecoveryAction> RecoveryEngine::getRecoveryHistory(const SystemState &state)
{
    return state.recoveryHistory;
}

// Clear Recovery History
void RecoveryEngine::clearRecoveryHistory(SystemState &state)
{
    state.recoveryHistory.clear();
    recoveryCounter = 0;
}