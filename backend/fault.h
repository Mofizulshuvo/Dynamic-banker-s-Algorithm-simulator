#ifndef FAULT_H
#define FAULT_H

#include "models.h"

class FaultEngine
{
public:

    FaultEngine();

    // Reduce available resource
    void injectFault(
        SystemState &state,
        int resourceID,
        int unitsLost
    );

    // Restore lost resource
    void restoreResource(
        SystemState &state,
        int resourceID,
        int units
    );

    // Print fault information
    void printFault(const FaultEvent &fault);
};

#endif