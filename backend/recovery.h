#ifndef RECOVERY_H
#define RECOVERY_H

#include "models.h"
#include <chrono>

class RecoveryEngine
{
private:
    int recoveryCounter;

public:
    RecoveryEngine();

    // Strategy 1: Restore Lost Resource
    RecoveryAction restoreResource(
        SystemState &state,
        int resourceID,
        int units
    );

    // Strategy 2: Suspend Process
    RecoveryAction suspendProcess(
        SystemState &state,
        int processID
    );

    // Strategy 3: Resume Process
    RecoveryAction resumeProcess(
        SystemState &state,
        int processID
    );

    // Strategy 4: Terminate Process
    RecoveryAction terminateProcess(
        SystemState &state,
        int processID
    );

    // Strategy 5: Manual Resource Reallocation
    RecoveryAction manualReallocation(
        SystemState &state,
        int fromProcess,
        int toProcess,
        int resourceID,
        int units
    );

    // Get recovery history
    vector<RecoveryAction> getRecoveryHistory(const SystemState &state);

    // Clear recovery history
    void clearRecoveryHistory(SystemState &state);

private:
    long long getCurrentTimestamp();
};

#endif