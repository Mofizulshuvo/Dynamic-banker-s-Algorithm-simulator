#include <iostream>
#include <fstream>
#include <vector>
#include "banker.h"
#include "fault.h"

using namespace std;

int main()
{
    ifstream in("data/input.json");
    ofstream out("data/output.json");

    int p, r;
    in >> p >> r;

    vector<vector<int>> alloc(p, vector<int>(r));
    vector<vector<int>> max(p, vector<int>(r));
    vector<int> avail(r);

    for(int i = 0; i < p; i++)
        for(int j = 0; j < r; j++)
            in >> alloc[i][j];

    for(int i = 0; i < p; i++)
        for(int j = 0; j < r; j++)
            in >> max[i][j];

    for(int i = 0; i < r; i++)
        in >> avail[i];

    vector<int> safeSeq;

    bool safe = isSafe(alloc, max, avail, p, r, safeSeq);

    out << "{\n";
    out << "\"safe\": " << (safe ? "true" : "false") << ",\n";

    out << "\"sequence\": [";
    for(int i = 0; i < safeSeq.size(); i++)
    {
        out << "\"P" << safeSeq[i] << "\"";
        if(i != safeSeq.size() - 1) out << ",";
    }
    out << "]\n}";

    out << "\n";
    out.close();
    in.close();

    return 0;
}