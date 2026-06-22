#include "fault.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

void injectFault(vector<int>& avail)
{
    srand(time(0));

    int idx = rand() % avail.size();
    int loss = 1 + rand() % 2;

    avail[idx] = max(0, avail[idx] - loss);

    cout << "Fault injected on Resource " << idx
         << " Loss: " << loss << endl;
}