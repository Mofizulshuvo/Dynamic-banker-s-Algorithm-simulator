#ifndef BANKER_H
#define BANKER_H
#include <bits/stdc++.h>
using namespace std;

bool isSafe(vector<vector<int>> alloc,
            vector<vector<int>> max,
            vector<int> avail,
            int p, int r);

void calculateNeed(vector<vector<int>> &need,
                   vector<vector<int>> max,
                   vector<vector<int>> alloc,
                   int p, int r);

void printMatrix(vector<vector<int>> mat, int p, int r, string name);

#endif