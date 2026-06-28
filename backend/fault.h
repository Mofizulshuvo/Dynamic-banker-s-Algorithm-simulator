#ifndef FAULT_H
#define FAULT_H

#include "models.h"
#include <chrono>

class FaultEngine
{
private:
    int faultCounter;

public:
    FaultEngine();

    // Inject resource loss fault
    FaultEvent injectResourceLoss(
        SystemState &state,
        int resourceID,
        int unitsLost
    );

    // Inject memory fragmentation fault
    FaultEvent injectMemoryFragmentation(
        SystemState &state,
        int resourceID,
        int unitsLost
    );

    // Inject hardware failure fault
    FaultEvent injectHardwareFailure(
        SystemState &state,
        int resourceID,
        int unitsLost
    );

    // Restore lost resource
    bool restoreResource(
        SystemState &state,
        int resourceID,
        int units
    );

    // Get fault history
    vector<FaultEvent> getFaultHistory(const SystemState &state);

    // Clear fault history
    void clearFaultHistory(SystemState &state);

private:
    long long getCurrentTimestamp();
};

#endif