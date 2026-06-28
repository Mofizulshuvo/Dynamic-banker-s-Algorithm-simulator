#include "banker.h"

#include <iostream>
#include <iomanip>

using namespace std;

// Constructor
Banker::Banker()
{
}

// Calculate Need Matrix
void Banker::calculateNeed(SystemState &state)
{
    state.need.assign(
        state.processCount,
        vector<int>(state.resourceCount)
    );

    for (int i = 0; i < state.processCount; i++)
    {
        for (int j = 0; j < state.resourceCount; j++)
        {
            state.need[i][j] =
                state.maximum[i][j] -
                state.allocation[i][j];
        }
    }
}

// Internal Safety Algorithm
bool Banker::safetyAlgorithm(const SystemState &state, vector<int> &safeSequence)
{
    vector<int> work = state.available;
    vector<bool> finish(state.processCount, false);
    safeSequence.clear();

    bool found;
    do
    {
        found = false;

        for (int i = 0; i < state.processCount; i++)
        {
            if (finish[i])
                continue;

            if (state.processes[i].status == SUSPENDED)
                continue;

            if (state.processes[i].status == TERMINATED)
                continue;

            bool possible = true;
            for (int j = 0; j < state.resourceCount; j++)
            {
                if (state.need[i][j] > work[j])
                {
                    possible = false;
                    break;
                }
            }

            if (possible)
            {
                for (int j = 0; j < state.resourceCount; j++)
                {
                    work[j] += state.allocation[i][j];
                }

                finish[i] = true;
                safeSequence.push_back(i);
                found = true;
            }
        }
    } while (found);

    for (int i = 0; i < state.processCount; i++)
    {
        if (state.processes[i].status == SUSPENDED)
            continue;

        if (state.processes[i].status == TERMINATED)
            continue;

        if (!finish[i])
            return false;
    }

    return true;
}

// Check Safe State
bool Banker::checkSafeState(SystemState &state)
{
    return safetyAlgorithm(state, state.safeSequence);
}

// Get Safe Sequence
vector<int> Banker::getSafeSequence(const SystemState &state)
{
    vector<int> seq;
    safetyAlgorithm(state, seq);
    return seq;
}

// Resource Request Algorithm
bool Banker::requestResources(SystemState &state, int processID, const vector<int> &request)
{
    if (processID < 0 || processID >= state.processCount)
        return false;

    if (!isValidRequest(state, processID, request))
        return false;

    if (!canAllocate(state, request))
        return false;

    // Pretend to allocate
    vector<int> tempAvailable = state.available;
    vector<vector<int>> tempAllocation = state.allocation;
    vector<vector<int>> tempNeed = state.need;

    for (int j = 0; j < state.resourceCount; j++)
    {
        tempAvailable[j] -= request[j];
        tempAllocation[processID][j] += request[j];
        tempNeed[processID][j] -= request[j];
    }

    // Create temporary state for safety check
    SystemState tempState = state;
    tempState.available = tempAvailable;
    tempState.allocation = tempAllocation;
    tempState.need = tempNeed;

    vector<int> tempSeq;
    if (safetyAlgorithm(tempState, tempSeq))
    {
        // Actually allocate
        state.available = tempAvailable;
        state.allocation = tempAllocation;
        state.need = tempNeed;
        return true;
    }

    return false;
}

// Validate Allocation Matrix
bool Banker::validateAllocation(const SystemState &state)
{
    for (int i = 0; i < state.processCount; i++)
    {
        for (int j = 0; j < state.resourceCount; j++)
        {
            if (state.allocation[i][j] < 0)
                return false;

            if (state.allocation[i][j] > state.maximum[i][j])
                return false;
        }
    }
    return true;
}

// Validate Maximum Matrix
bool Banker::validateMaximum(const SystemState &state)
{
    for (int i = 0; i < state.processCount; i++)
    {
        for (int j = 0; j < state.resourceCount; j++)
        {
            if (state.maximum[i][j] < 0)
                return false;
        }
    }
    return true;
}

// Check if Request is Valid (Request <= Need)
bool Banker::isValidRequest(const SystemState &state, int processID, const vector<int> &request)
{
    for (int j = 0; j < state.resourceCount; j++)
    {
        if (request[j] < 0)
            return false;

        if (request[j] > state.need[processID][j])
            return false;
    }
    return true;
}

// Check if Allocation is Possible (Request <= Available)
bool Banker::canAllocate(const SystemState &state, const vector<int> &request)
{
    for (int j = 0; j < state.resourceCount; j++)
    {
        if (request[j] > state.available[j])
            return false;
    }
    return true;
}