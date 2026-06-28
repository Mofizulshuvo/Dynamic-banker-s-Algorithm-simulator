#ifndef BANKER_H
#define BANKER_H

#include <vector>

using namespace std;

class Banker
{
private:

    int processCount;
    int resourceCount;

    vector<vector<int>> allocation;
    vector<vector<int>> maximum;
    vector<vector<int>> need;
    vector<int> available;

    // Suspended process list
    vector<bool> suspended;

    // Safe sequence
    vector<int> safeSequence;

public:

    Banker();

    // Load all data
    void setData(
        int p,
        int r,
        vector<vector<int>> alloc,
        vector<vector<int>> max,
        vector<int> avail
    );

    // Need Matrix
    void calculateNeed();

    // Safe State Check
    bool checkSafeState();

    // Display Safe Sequence
    void printSafeSequence();

    // Getters
    vector<int> getSafeSequence();

    vector<vector<int>> getNeed();

    // Suspend / Resume
    void suspendProcess(int processID);

    void resumeProcess(int processID);

    bool isSuspended(int processID);

};

#endif