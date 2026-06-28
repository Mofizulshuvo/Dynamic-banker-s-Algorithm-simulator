#include "simulation.h"

#include <iostream>
#include <iomanip>

using namespace std;

// Constructor
Simulation::Simulation()
{
    processCount = 0;
    resourceCount = 0;

    safeState = false;
    simulationRunning = false;
    simulationPaused = false;
    faultOccurred = false;
}

// Initialize System
void Simulation::initialize()
{
    cout << "Dynamic Banker's Algorithm Simulator\n\n";

    cout << "Enter Number of Processes: ";
    cin >> processCount;

    cout << "Enter Number of Resource Types: ";
    cin >> resourceCount;

    allocation.resize(processCount, vector<int>(resourceCount));
    maximum.resize(processCount, vector<int>(resourceCount));
    need.resize(processCount, vector<int>(resourceCount));
    available.resize(resourceCount);

    cout << "\nEnter Allocation Matrix\n";

    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            cin >> allocation[i][j];
        }
    }

    cout << "\nEnter Maximum Matrix\n";

    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            cin >> maximum[i][j];
        }
    }

    cout << "\nEnter Available Resources\n";

    for (int i = 0; i < resourceCount; i++)
    {
        cin >> available[i];
    }

    calculateNeed();
}


// Need Matrix
void Simulation::calculateNeed()
{
    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            need[i][j] = maximum[i][j] - allocation[i][j];
        }
    }
}

// Display Current System
void Simulation::displaySystem()
{
    cout << "Current System State";
    cout << "\nAllocation Matrix\n";

    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            cout << setw(5) << allocation[i][j];
        }
        cout << endl;
    }

    cout << "\nMaximum Matrix\n";

    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            cout << setw(5) << maximum[i][j];
        }
        cout << endl;
    }

    cout << "\nNeed Matrix\n";

    for (int i = 0; i < processCount; i++)
    {
        for (int j = 0; j < resourceCount; j++)
        {
            cout << setw(5) << need[i][j];
        }
        cout << endl;
    }

    cout << "\nAvailable Resources\n";

    for (int i = 0; i < resourceCount; i++)
    {
        cout << setw(5) << available[i];
    }

    cout << endl;
}

// Start Simulation
void Simulation::startSimulation()
{
    simulationRunning = true;
    simulationPaused = false;

    cout << "\nSimulation Started...\n";
}


// Pause
void Simulation::pauseSimulation()
{
    simulationPaused = true;

    cout << "\nSimulation Paused.\n";
}


// Resume
void Simulation::resumeSimulation()
{
    simulationPaused = false;

    cout << "\nSimulation Resumed.\n";
}


// Fault
void Simulation::injectFault()
{
    faultOccurred = true;

    cout << "\nRuntime Fault Injected.\n";
}


// Recovery
void Simulation::recoverSystem()
{
    faultOccurred = false;

    cout << "\nRecovery Completed.\n";
}


// Status
bool Simulation::isRunning()
{
    return simulationRunning;
}

bool Simulation::isPaused()
{
    return simulationPaused;
}