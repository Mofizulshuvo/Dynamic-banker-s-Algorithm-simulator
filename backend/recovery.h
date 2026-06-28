#ifndef RECOVERY_H
#define RECOVERY_H

#include "models.h"

class RecoveryEngine
{
public:

    RecoveryEngine();

    // Strategy 1
    bool restoreResource(
        SystemState &state,
        int resourceID,
        int units
    );

    // Strategy 2
    bool suspendProcess(
        SystemState &state,
        int processID
    );

    // Strategy 3
    bool resumeProcess(
        SystemState &state,
        int processID
    );

    // Strategy 4
    bool terminateProcess(
        SystemState &state,
        int processID
    );

    // Strategy 5
    bool manualReallocation(
        SystemState &state,
        int fromProcess,
        int toProcess,
        int resourceID,
        int units
    );

    // Print Recovery Result
    void printRecovery(const RecoveryAction &action);

};

#endif