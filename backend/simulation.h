#ifndef SIMULATION_H
#define SIMULATION_H

#include <vector>
#include <string>

using namespace std;

class Simulation
{
private:

    // Number of processes and resources
    int processCount;
    int resourceCount;

    // System matrices
    vector<vector<int>> allocation;
    vector<vector<int>> maximum;
    vector<vector<int>> need;
    vector<int> available;

    // Simulation status
    bool safeState;
    bool simulationRunning;
    bool simulationPaused;
    bool faultOccurred;

    // Safe execution order
    vector<int> safeSequence;

public:

    // Constructor
    Simulation();

    // Initializ
    void initialize();

    //  Display 
    void displaySystem();

    //  Need Matrix 
    void calculateNeed();

    //  Simulation 
    void startSimulation();

    void pauseSimulation();

    void resumeSimulation();

    //  Fault 
    void injectFault();

    //  Recovery 
    void recoverSystem();

    //  Utility 
    bool isRunning();

    bool isPaused();

};

#endif