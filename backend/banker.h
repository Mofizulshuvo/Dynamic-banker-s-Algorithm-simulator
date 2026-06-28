#ifndef BANKER_H
#define BANKER_H

#include "models.h"

class Banker
{
public:
    Banker();

    // Calculate Need Matrix
    void calculateNeed(SystemState &state);

    // Check Safe State
    bool checkSafeState(SystemState &state);

    // Resource Request Algorithm
    bool requestResources(SystemState &state, int processID, const vector<int> &request);

    // Validate allocation matrix
    bool validateAllocation(const SystemState &state);

    // Validate maximum matrix
    bool validateMaximum(const SystemState &state);

    // Check if request is valid (request <= need)
    bool isValidRequest(const SystemState &state, int processID, const vector<int> &request);

    // Check if allocation is possible (request <= available)
    bool canAllocate(const SystemState &state, const vector<int> &request);

    // Get safe sequence
    vector<int> getSafeSequence(const SystemState &state);

private:
    // Internal safety check algorithm
    bool safetyAlgorithm(const SystemState &state, vector<int> &safeSequence);
};

#endif