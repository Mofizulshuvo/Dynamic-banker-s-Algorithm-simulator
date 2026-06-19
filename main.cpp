#include <iostream>
#include <vector>
#include "banker.h"
#include "fault.h"

using namespace std;

int main()
{
    int p, r;

    cout << "Enter number of processes: ";
    cin >> p;

    cout << "Enter number of resources: ";
    cin >> r;

    vector<vector<int>> alloc(p, vector<int>(r));
    vector<vector<int>> max(p, vector<int>(r));
    vector<int> avail(r);

    cout << "Enter Allocation Matrix:\n";
    for(int i = 0; i < p; i++)
        for(int j = 0; j < r; j++)
            cin >> alloc[i][j];

    cout << "Enter Max Matrix:\n";
    for(int i = 0; i < p; i++)
        for(int j = 0; j < r; j++)
            cin >> max[i][j];

    cout << "Enter Available Vector:\n";
    for(int j = 0; j < r; j++)
        cin >> avail[j];

    if(isSafe(alloc, max, avail, p, r))
        cout << "\nSystem is in SAFE state\n";
    else
        cout << "\nSystem is UNSAFE\n";

    injectFault(avail);

    if(isSafe(alloc, max, avail, p, r))
        cout << "\nAfter Fault: SAFE state\n";
    else
        cout << "\nAfter Fault: DEGENERATED MODE\n";

    return 0;
}