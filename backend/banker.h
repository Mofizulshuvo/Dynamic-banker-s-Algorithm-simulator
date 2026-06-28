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

    // Print Need Matrix
    void printNeedMatrix(const SystemState &state);

    // Print Safe Sequence
    void printSafeSequence(const SystemState &state);
};

#endif