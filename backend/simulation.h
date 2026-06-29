#ifndef SIMULATION_H
#define SIMULATION_H

#include "models.h"
#include "banker.h"
#include "fault.h"
#include "recovery.h"
#include <chrono>

class Simulation
{
private:
    SystemState state;
    vector<vector<int>> initialAllocation;
    vector<vector<int>> initialMaximum;
    vector<int> initialAvailable;
    vector<int> initialTotalResources;
    Banker banker;
    FaultEngine faultEngine;
    RecoveryEngine recoveryEngine;
    int timelineCounter;

public:
    Simulation();

    // Initialize System with given parameters
    bool initialize(int processCount, int resourceCount,
                    const vector<vector<int>>& allocation,
                    const vector<vector<int>>& maximum,
                    const vector<int>& available);

    // Initialize System with total resources
    bool initializeWithTotal(int processCount, int resourceCount,
                            const vector<vector<int>>& allocation,
                            const vector<vector<int>>& maximum,
                            const vector<int>& available,
                            const vector<int>& totalResources);

    // Get current state
    SystemState& getState();

    // Reset simulation
    void reset();

    // Run complete simulation
    bool run();

    // Run one step of simulation
    bool runOneStep();

    // Pause simulation
    void pause();

    // Resume simulation
    void resume();

    // Set simulation speed (milliseconds per step)
    void setSimulationSpeed(int speed);

    // Get simulation speed
    int getSimulationSpeed();

    // Inject fault during runtime
    FaultEvent injectFault(FaultType type, int resourceID, int unitsLost);

    // Apply recovery
    RecoveryAction recover(RecoveryType type, int processID = -1,
                         int resourceID = -1, int units = 0,
                         int fromProcess = -1, int toProcess = -1);

    // Add timeline event
    void addTimelineEvent(const string& event, int processID = -1);

    // Get timeline
    vector<TimelineEvent> getTimeline();

    // Clear timeline
    void clearTimeline();

    // Check if system is safe
    bool isSafe();

    // Get safe sequence
    vector<int> getSafeSequence();

    // Validate current state
    bool validateState();

private:
    long long getCurrentTimestamp();
};

#endif
