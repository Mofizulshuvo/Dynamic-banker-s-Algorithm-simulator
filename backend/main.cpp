#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "banker.h"
#include "fault.h"

using namespace std;

// Simple JSON parser for input
vector<vector<int>> parseMatrix(string json) {
    vector<vector<int>> result;
    // Remove brackets and split by rows
    // This is a simplified parser - assumes well-formed JSON
    return result;
}

int main()
{
    // Read from input.json
    ifstream in("data/input.json");
    ofstream out("data/output.json");

    if (!in.is_open()) {
        cerr << "Error: Cannot open input.json" << endl;
        return 1;
    }

    // Parse JSON input (simplified - reading space-separated values)
    // Format: processes resources allocation_matrix max_matrix available_vector
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

    // Write JSON output
    out << "{\n";
    out << "  \"safe\": " << (safe ? "true" : "false") << ",\n";
    out << "  \"sequence\": [";
    for(int i = 0; i < safeSeq.size(); i++)
    {
        out << "\"P" << safeSeq[i] << "\"";
        if(i != safeSeq.size() - 1) out << ", ";
    }
    out << "],\n";
    out << "  \"message\": \"" << (safe ? "SAFE - No deadlock possible" : "UNSAFE - Deadlock risk detected") << "\"\n";
    out << "}\n";

    out.close();
    in.close();

    return 0;
}