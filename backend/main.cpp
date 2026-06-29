#include "server.h"
#include <iostream>

using namespace std;

int main()
{
    int port = 8080;
    
    cout << "========================================" << endl;
    cout << "Dynamic Banker's Algorithm Simulator" << endl;
    cout << "========================================" << endl;
    cout << endl;
    
    // Start the HTTP server
    startServer(port);
    
    return 0;
}
