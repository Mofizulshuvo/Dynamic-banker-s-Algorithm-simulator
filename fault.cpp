#include "fault.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

void injectFault(vector<int> &avail)
{
    srand(time(0));

    int r = rand() % avail.size();
    int loss = 1 + rand() % 3;

    cout << "\nFAULT INJECTED\n";
    cout << "Resource Index: " << r << endl;
    cout << "Units Lost: " << loss << endl;

    avail[r] = max(0, avail[r] - loss);
}