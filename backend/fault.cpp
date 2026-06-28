#include "fault.h"

#include <iostream>

using namespace std;

// Constructor
FaultEngine::FaultEngine()
{
}


// Inject Fault
void FaultEngine::injectFault(
    SystemState &state,
    int resourceID,
    int unitsLost
)
{
    if(resourceID < 0 || resourceID >= state.resourceCount)
        return;

    if(unitsLost < 0)
        return;

    if(unitsLost > state.available[resourceID])
        unitsLost = state.available[resourceID];

    state.available[resourceID] -= unitsLost;

    state.simulationState = FAULT;
}


// Restore Resource
void FaultEngine::restoreResource(
    SystemState &state,
    int resourceID,
    int units
)
{
    if(resourceID < 0 || resourceID >= state.resourceCount)
        return;

    if(units < 0)
        return;

    state.available[resourceID] += units;
}


// Print Fault
void FaultEngine::printFault(const FaultEvent &fault)
{
    cout << "\nFAULT\n";

    cout << "Resource ID : "
         << fault.resourceID << endl;

    cout << "Units Lost  : "
         << fault.unitsLost << endl;

    cout << "Description : "
         << fault.description << endl;

    cout << "\n";
}