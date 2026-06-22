#ifndef BANKER_H
#define BANKER_H

#include <vector>
using namespace std;

void calculateNeed(vector<vector<int>>& need,
                   vector<vector<int>> max,
                   vector<vector<int>> alloc,
                   int p, int r);

bool isSafe(vector<vector<int>> alloc,
            vector<vector<int>> max,
            vector<int> avail,
            int p, int r,
            vector<int>& safeSeq);

#endif