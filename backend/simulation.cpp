#include "simulation.h"

#include <iostream>
#include <iomanip>

using namespace std;

// Constructor
Simulation::Simulation()
{
    state.processCount = 0;
    state.resourceCount = 0;
    state.simulationState = IDLE;
    state.currentStep = 0;
    state.simulationFinished = false;
    state.simulationSpeed = 1000; // Default 1 second per step
    timelineCounter = 0;
}

// Get Current Timestamp
long long Simulation::getCurrentTimestamp()
{
    auto now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return chrono::duration_cast<chrono::milliseconds>(duration).count();
}

// Initialize System
bool Simulation::initialize(int processCount, int resourceCount,
                           const vector<vector<int>>& allocation,
                           const vector<vector<int>>& maximum,
                           const vector<int>& available)
{
    if (processCount <= 0 || resourceCount <= 0)
        return false;

    if (allocation.size() != processCount || maximum.size() != processCount)
        return false;

    if (available.size() != resourceCount)
        return false;

    state.processCount = processCount;
    state.resourceCount = resourceCount;
    state.allocation = allocation;
    state.maximum = maximum;
    state.available = available;
    state.totalResources = available;
    initialAllocation = allocation;
    initialMaximum = maximum;
    initialAvailable = available;
    initialTotalResources = available;

    // Initialize need matrix
    state.need.assign(processCount, vector<int>(resourceCount));
    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            state.need[i][j] = state.maximum[i][j] - state.allocation[i][j];
        }
    }

    // Initialize processes
    state.processes.clear();
    for (int i = 0; i < processCount; i++)
    {
        Process p;
        p.id = i;
        p.status = WAITING;
        p.progress = 0;
        state.processes.push_back(p);
    }

    state.currentStep = 0;
    state.simulationFinished = false;
    state.simulationState = INITIALIZED;
    state.faultHistory.clear();
    state.recoveryHistory.clear();
    state.timeline.clear();
    timelineCounter = 0;

    addTimelineEvent("System initialized");

    return true;
}

// Initialize System with Total Resources
bool Simulation::initializeWithTotal(int processCount, int resourceCount,
                                     const vector<vector<int>>& allocation,
                                     const vector<vector<int>>& maximum,
                                     const vector<int>& available,
                                     const vector<int>& totalResources)
{
    if (!initialize(processCount, resourceCount, allocation, maximum, available))
        return false;

    state.totalResources = totalResources;
    initialTotalResources = totalResources;
    return true;
}

// Get Current State
SystemState& Simulation::getState()
{
    return state;
}

// Reset Simulation
void Simulation::reset()
{
    state.currentStep = 0;
    state.simulationFinished = false;
    state.simulationState = INITIALIZED;
    state.faultHistory.clear();
    state.recoveryHistory.clear();
    state.timeline.clear();
    timelineCounter = 0;
    state.allocation = initialAllocation;
    state.maximum = initialMaximum;
    state.available = initialAvailable;
    state.totalResources = initialTotalResources;
    banker.calculateNeed(state);

    // Reset process statuses
    for (auto& process : state.processes)
    {
        process.status = WAITING;
        process.progress = 0;
    }

    addTimelineEvent("Simulation reset");
}

// Run Complete Simulation
bool Simulation::run()
{
    if (state.simulationState != INITIALIZED && state.simulationState != PAUSED)
        return false;

    banker.calculateNeed(state);

    if (!banker.checkSafeState(state))
    {
        state.simulationState = UNSAFE;
        addTimelineEvent("System is in unsafe state");
        return false;
    }

    state.simulationState = RUNNING_STATE;
    addTimelineEvent("Simulation started");

    return true;
}

// Run One Step
bool Simulation::runOneStep()
{
    if (state.simulationState != RUNNING_STATE)
        return false;

    if (state.simulationFinished)
        return false;

    if (state.currentStep >= state.safeSequence.size())
    {
        state.simulationFinished = true;
        state.simulationState = COMPLETED;
        addTimelineEvent("Simulation completed");
        return false;
    }

    int processID = state.safeSequence[state.currentStep];

    // Skip suspended or terminated processes
    if (state.processes[processID].status == SUSPENDED)
    {
        addTimelineEvent("Skipped suspended process P" + to_string(processID), processID);
        state.currentStep++;
        return true;
    }

    if (state.processes[processID].status == TERMINATED)
    {
        addTimelineEvent("Skipped terminated process P" + to_string(processID), processID);
        state.currentStep++;
        return true;
    }

    // Run process
    state.processes[processID].status = RUNNING;
    state.processes[processID].progress = 50;
    addTimelineEvent("P" + to_string(processID) + " started execution", processID);

    // Release allocated resources
    for (int i = 0; i < state.resourceCount; i++)
    {
        state.available[i] += state.allocation[processID][i];
        state.allocation[processID][i] = 0;
        state.need[processID][i] = 0;
    }

    state.processes[processID].status = FINISHED;
    state.processes[processID].progress = 100;
    addTimelineEvent("P" + to_string(processID) + " finished and released resources", processID);

    state.currentStep++;

    // Check if simulation is complete
    if (state.currentStep == state.safeSequence.size())
    {
        state.simulationFinished = true;
        state.simulationState = COMPLETED;
        addTimelineEvent("All processes completed successfully");
    }

    return true;
}

// Pause Simulation
void Simulation::pause()
{
    if (state.simulationState == RUNNING_STATE)
    {
        state.simulationState = PAUSED;
        addTimelineEvent("Simulation paused");
    }
}

// Resume Simulation
void Simulation::resume()
{
    if (state.simulationState == PAUSED)
    {
        state.simulationState = RUNNING_STATE;
        addTimelineEvent("Simulation resumed");
    }
}

// Set Simulation Speed
void Simulation::setSimulationSpeed(int speed)
{
    if (speed >= 100 && speed <= 10000)
    {
        state.simulationSpeed = speed;
    }
}

// Get Simulation Speed
int Simulation::getSimulationSpeed()
{
    return state.simulationSpeed;
}

// Inject Fault
FaultEvent Simulation::injectFault(FaultType type, int resourceID, int unitsLost)
{
    FaultEvent fault;

    if (state.simulationState != RUNNING_STATE)
    {
        fault.description = "Simulation is not running";
        return fault;
    }

    switch (type)
    {
        case RESOURCE_LOSS:
            fault = faultEngine.injectResourceLoss(state, resourceID, unitsLost);
            break;
        case MEMORY_FRAGMENTATION:
            fault = faultEngine.injectMemoryFragmentation(state, resourceID, unitsLost);
            break;
        case HARDWARE_FAILURE:
            fault = faultEngine.injectHardwareFailure(state, resourceID, unitsLost);
            break;
    }

    addTimelineEvent("Fault injected: " + fault.description);

    // Recalculate safety
    banker.calculateNeed(state);
    bool safe = banker.checkSafeState(state);

    if (safe)
    {
        state.simulationState = RUNNING_STATE;
        addTimelineEvent("System remains safe after fault");
    }
    else
    {
        state.simulationState = UNSAFE;
        addTimelineEvent("System became unsafe after fault");
    }

    return fault;
}

// Apply Recovery
RecoveryAction Simulation::recover(RecoveryType type, int processID,
                                  int resourceID, int units,
                                  int fromProcess, int toProcess)
{
    RecoveryAction action;

    switch (type)
    {
        case RESTORE_RESOURCE:
            action = recoveryEngine.restoreResource(state, resourceID, units);
            break;
        case SUSPEND_PROCESS:
            action = recoveryEngine.suspendProcess(state, processID);
            break;
        case RESUME_PROCESS:
            action = recoveryEngine.resumeProcess(state, processID);
            break;
        case TERMINATE_PROCESS:
            action = recoveryEngine.terminateProcess(state, processID);
            break;
        case MANUAL_REALLOCATION:
            action = recoveryEngine.manualReallocation(state, fromProcess, toProcess, resourceID, units);
            break;
    }

    addTimelineEvent("Recovery applied: " + action.message);

    // Recalculate safety
    banker.calculateNeed(state);
    bool safe = banker.checkSafeState(state);

    if (safe)
    {
        state.simulationState = RUNNING_STATE;
        addTimelineEvent("System recovered to safe state");
    }
    else
    {
        state.simulationState = UNSAFE;
        addTimelineEvent("System still unsafe after recovery");
    }

    return action;
}

// Add Timeline Event
void Simulation::addTimelineEvent(const string& event, int processID)
{
    TimelineEvent te;
    te.id = timelineCounter++;
    te.event = event;
    te.processID = processID;
    te.available = state.available;
    te.safe = (state.simulationState == SAFE || state.simulationState == RUNNING_STATE || state.simulationState == INITIALIZED);
    te.timestamp = getCurrentTimestamp();
    state.timeline.push_back(te);
}

// Get Timeline
vector<TimelineEvent> Simulation::getTimeline()
{
    return state.timeline;
}

// Clear Timeline
void Simulation::clearTimeline()
{
    state.timeline.clear();
    timelineCounter = 0;
}

// Check if System is Safe
bool Simulation::isSafe()
{
    banker.calculateNeed(state);
    return banker.checkSafeState(state);
}

// Get Safe Sequence
vector<int> Simulation::getSafeSequence()
{
    banker.calculateNeed(state);
    return banker.getSafeSequence(state);
}

// Validate Current State
bool Simulation::validateState()
{
    return banker.validateAllocation(state) && banker.validateMaximum(state);
}
