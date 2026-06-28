#include "recovery.h"

#include <iostream>

using namespace std;

// Constructor
RecoveryEngine::RecoveryEngine()
{
}

// Restore Resource
bool RecoveryEngine::restoreResource(
    SystemState &state,
    int resourceID,
    int units
)
{
    if(resourceID < 0 || resourceID >= state.resourceCount)
        return false;

    if(units <= 0)
        return false;

    state.available[resourceID] += units;

    return true;
}

// Suspend Process
bool RecoveryEngine::suspendProcess(
    SystemState &state,
    int processID
)
{
    if(processID < 0 || processID >= state.processCount)
        return false;

    state.processes[processID].status = SUSPENDED;

    return true;
}

// Resume Process
bool RecoveryEngine::resumeProcess(
    SystemState &state,
    int processID
)
{
    if(processID < 0 || processID >= state.processCount)
        return false;

    state.processes[processID].status = WAITING;

    return true;
}

// Terminate Process
bool RecoveryEngine::terminateProcess(
    SystemState &state,
    int processID
)
{
    if(processID < 0 || processID >= state.processCount)
        return false;

    state.processes[processID].status = TERMINATED;

    // Release allocated resources
    for(int i = 0; i < state.resourceCount; i++)
    {
        state.available[i] += state.allocation[processID][i];
        state.allocation[processID][i] = 0;
        state.need[processID][i] = 0;
    }

    return true;
}


// Manual Resource Reallocation
bool RecoveryEngine::manualReallocation(
    SystemState &state,
    int fromProcess,
    int toProcess,
    int resourceID,
    int units
)
{
    if(fromProcess < 0 || fromProcess >= state.processCount)
        return false;

    if(toProcess < 0 || toProcess >= state.processCount)
        return false;

    if(resourceID < 0 || resourceID >= state.resourceCount)
        return false;

    if(units <= 0)
        return false;

    if(state.allocation[fromProcess][resourceID] < units)
        return false;

    state.allocation[fromProcess][resourceID] -= units;
    state.allocation[toProcess][resourceID] += units;

    return true;
}

// Print Recovery
void RecoveryEngine::printRecovery(
    const RecoveryAction &action
)
{
    cout << "\n========== RECOVERY ==========\n";

    cout << "Strategy : ";

    switch(action.type)
    {
        case RESTORE_RESOURCE:
            cout << "Restore Resource";
            break;

        case SUSPEND_PROCESS:
            cout << "Suspend Process";
            break;

        case WAIT_FOR_RESOURCE:
            cout << "Wait";
            break;

        case TERMINATE_PROCESS:
            cout << "Terminate Process";
            break;

        case MANUAL_REALLOCATION:
            cout << "Manual Reallocation";
            break;
    }

    cout << endl;

    cout << "Success : "
         << (action.success ? "YES" : "NO")
         << endl;

    cout << "Message : "
         << action.message
         << endl;

    cout << "\n";
}