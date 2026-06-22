#include "banker.h"

void calculateNeed(vector<vector<int>>& need,
                   vector<vector<int>> max,
                   vector<vector<int>> alloc,
                   int p, int r)
{
    for(int i = 0; i < p; i++)
        for(int j = 0; j < r; j++)
            need[i][j] = max[i][j] - alloc[i][j];
}

bool isSafe(vector<vector<int>> alloc,
            vector<vector<int>> max,
            vector<int> avail,
            int p, int r,
            vector<int>& safeSeq)
{
    vector<vector<int>> need(p, vector<int>(r));
    calculateNeed(need, max, alloc, p, r);

    vector<bool> finish(p, false);
    vector<int> work = avail;

    int count = 0;

    while(count < p)
    {
        bool found = false;

        for(int i = 0; i < p; i++)
        {
            if(!finish[i])
            {
                bool canRun = true;

                for(int j = 0; j < r; j++)
                {
                    if(need[i][j] > work[j])
                    {
                        canRun = false;
                        break;
                    }
                }

                if(canRun)
                {
                    for(int j = 0; j < r; j++)
                        work[j] += alloc[i][j];

                    safeSeq.push_back(i);
                    finish[i] = true;
                    found = true;
                    count++;
                }
            }
        }

        if(!found)
            return false;
    }

    return true;
}