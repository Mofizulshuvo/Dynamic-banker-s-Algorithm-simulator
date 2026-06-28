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

// Check Safe State
bool Banker::checkSafeState(SystemState &state)
{
    state.safeSequence.clear();

    vector<int> work = state.available;

    vector<bool> finish(state.processCount, false);

    bool found;

    do
    {
        found = false;

        for (int i = 0; i < state.processCount; i++)
        {
            // Skip Finished
            if (finish[i])
                continue;

            // Skip Suspended
            if (state.processes[i].status == SUSPENDED)
                continue;

            // Skip Terminated
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

                state.safeSequence.push_back(i);

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

// Print Need Matrix
void Banker::printNeedMatrix(const SystemState &state)
{
    cout << "\nNeed Matrix\n\n";

    for (int i = 0; i < state.processCount; i++)
    {
        for (int j = 0; j < state.resourceCount; j++)
        {
            cout << setw(5) << state.need[i][j];
        }

        cout << endl;
    }
}


// Print Safe Sequence
void Banker::printSafeSequence(const SystemState &state)
{
    cout << "\nSafe Sequence : ";

    for (int i = 0; i < state.safeSequence.size(); i++)
    {
        cout << "P" << state.safeSequence[i];

        if (i != state.safeSequence.size() - 1)
        {
            cout << " -> ";
        }
    }

    cout << endl;
}