#include "fault.h"

#include <iostream>

using namespace std;

// Constructor
FaultEngine::FaultEngine()
{
    faultCounter = 0;
}

// Get Current Timestamp
long long FaultEngine::getCurrentTimestamp()
{
    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(duration).count();
}

// Inject Resource Loss Fault
FaultEvent FaultEngine::injectResourceLoss(
    SystemState &state,
    int resourceID,
    int unitsLost
)
{
    FaultEvent fault;
    fault.id = faultCounter++;
    fault.type = RESOURCE_LOSS;
    fault.resourceID = resourceID;
    fault.unitsLost = unitsLost;
    fault.timestamp = getCurrentTimestamp();

    if (resourceID < 0 || resourceID >= state.resourceCount)
    {
        fault.description = "Invalid resource ID";
        return fault;
    }

    if (unitsLost < 0)
    {
        fault.description = "Invalid units lost";
        return fault;
    }

    if (unitsLost > state.available[resourceID])
        unitsLost = state.available[resourceID];

    state.available[resourceID] -= unitsLost;
    fault.unitsLost = unitsLost;
    fault.description = "Resource loss: Resource " + to_string(resourceID) + " lost " + to_string(unitsLost) + " units";

    state.faultHistory.push_back(fault);
    state.simulationState = FAULT;

    return fault;
}

// Inject Memory Fragmentation Fault
FaultEvent FaultEngine::injectMemoryFragmentation(
    SystemState &state,
    int resourceID,
    int unitsLost
)
{
    FaultEvent fault;
    fault.id = faultCounter++;
    fault.type = MEMORY_FRAGMENTATION;
    fault.resourceID = resourceID;
    fault.unitsLost = unitsLost;
    fault.timestamp = getCurrentTimestamp();

    if (resourceID < 0 || resourceID >= state.resourceCount)
    {
        fault.description = "Invalid resource ID";
        return fault;
    }

    if (unitsLost < 0)
    {
        fault.description = "Invalid units lost";
        return fault;
    }

    if (unitsLost > state.available[resourceID])
        unitsLost = state.available[resourceID];

    state.available[resourceID] -= unitsLost;
    fault.unitsLost = unitsLost;
    fault.description = "Memory fragmentation: Resource " + to_string(resourceID) + " fragmented " + to_string(unitsLost) + " units";

    state.faultHistory.push_back(fault);
    state.simulationState = FAULT;

    return fault;
}

// Inject Hardware Failure Fault
FaultEvent FaultEngine::injectHardwareFailure(
    SystemState &state,
    int resourceID,
    int unitsLost
)
{
    FaultEvent fault;
    fault.id = faultCounter++;
    fault.type = HARDWARE_FAILURE;
    fault.resourceID = resourceID;
    fault.unitsLost = unitsLost;
    fault.timestamp = getCurrentTimestamp();

    if (resourceID < 0 || resourceID >= state.resourceCount)
    {
        fault.description = "Invalid resource ID";
        return fault;
    }

    if (unitsLost < 0)
    {
        fault.description = "Invalid units lost";
        return fault;
    }

    if (unitsLost > state.available[resourceID])
        unitsLost = state.available[resourceID];

    state.available[resourceID] -= unitsLost;
    fault.unitsLost = unitsLost;
    fault.description = "Hardware failure: Resource " + to_string(resourceID) + " failed " + to_string(unitsLost) + " units";

    state.faultHistory.push_back(fault);
    state.simulationState = FAULT;

    return fault;
}

// Restore Resource
bool FaultEngine::restoreResource(
    SystemState &state,
    int resourceID,
    int units
)
{
    if (resourceID < 0 || resourceID >= state.resourceCount)
        return false;

    if (units < 0)
        return false;

    state.available[resourceID] += units;
    return true;
}

// Get Fault History
vector<FaultEvent> FaultEngine::getFaultHistory(const SystemState &state)
{
    return state.faultHistory;
}

// Clear Fault History
void FaultEngine::clearFaultHistory(SystemState &state)
{
    state.faultHistory.clear();
    faultCounter = 0;
}