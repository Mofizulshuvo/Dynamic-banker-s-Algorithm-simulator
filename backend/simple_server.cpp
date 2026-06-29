#include "server.h"
#include "simulation.h"
#include "external/json.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using json = nlohmann::json;
using namespace std;

// Global simulation instance
unique_ptr<Simulation> g_simulation;
mutex g_simulationMutex;

// Helper function to convert ProcessStatus to string
string processStatusToString(ProcessStatus status)
{
    switch (status)
    {
        case WAITING: return "WAITING";
        case RUNNING: return "RUNNING";
        case FINISHED: return "FINISHED";
        case SUSPENDED: return "SUSPENDED";
        case TERMINATED: return "TERMINATED";
        default: return "UNKNOWN";
    }
}

// Helper function to convert SimulationState to string
string simulationStateToString(SimulationState state)
{
    switch (state)
    {
        case IDLE: return "IDLE";
        case INITIALIZED: return "INITIALIZED";
        case SAFE: return "SAFE";
        case UNSAFE: return "UNSAFE";
        case RUNNING_STATE: return "RUNNING";
        case PAUSED: return "PAUSED";
        case FAULT: return "FAULT";
        case RECOVERY: return "RECOVERY";
        case COMPLETED: return "COMPLETED";
        default: return "UNKNOWN";
    }
}

// Helper function to convert FaultType to string
string faultTypeToString(FaultType type)
{
    switch (type)
    {
        case RESOURCE_LOSS: return "RESOURCE_LOSS";
        case MEMORY_FRAGMENTATION: return "MEMORY_FRAGMENTATION";
        case HARDWARE_FAILURE: return "HARDWARE_FAILURE";
        default: return "UNKNOWN";
    }
}

// Helper function to convert RecoveryType to string
string recoveryTypeToString(RecoveryType type)
{
    switch (type)
    {
        case RESTORE_RESOURCE: return "RESTORE_RESOURCE";
        case SUSPEND_PROCESS: return "SUSPEND_PROCESS";
        case RESUME_PROCESS: return "RESUME_PROCESS";
        case TERMINATE_PROCESS: return "TERMINATE_PROCESS";
        case MANUAL_REALLOCATION: return "MANUAL_REALLOCATION";
        default: return "UNKNOWN";
    }
}

// Convert system state to JSON
json systemStateToJson(const SystemState& state)
{
    json j;
    j["simulationState"] = simulationStateToString(state.simulationState);
    j["processCount"] = state.processCount;
    j["resourceCount"] = state.resourceCount;
    
    // Convert processes
    j["processes"] = json::array();
    for (const auto& proc : state.processes)
    {
        json p;
        p["id"] = proc.id;
        p["status"] = processStatusToString(proc.status);
        p["progress"] = proc.progress;
        j["processes"].push_back(p);
    }
    
    // Convert matrices
    j["allocation"] = state.allocation;
    j["maximum"] = state.maximum;
    j["need"] = state.need;
    j["available"] = state.available;
    j["totalResources"] = state.totalResources;
    j["safeSequence"] = state.safeSequence;
    
    // Convert timeline
    j["timeline"] = json::array();
    for (const auto& event : state.timeline)
    {
        json t;
        t["id"] = event.id;
        t["event"] = event.event;
        t["processID"] = event.processID;
        t["available"] = event.available;
        t["safe"] = event.safe;
        t["timestamp"] = event.timestamp;
        j["timeline"].push_back(t);
    }
    
    return j;
}

// HTTP response helper
string statusText(int status)
{
    switch (status)
    {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

string createResponse(const string& body, int status = 200, const string& contentType = "application/json")
{
    return "HTTP/1.1 " + to_string(status) + " " + statusText(status) + "\r\n"
           "Content-Type: " + contentType + "\r\n"
           "Access-Control-Allow-Origin: *\r\n"
           "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
           "Access-Control-Allow-Headers: Content-Type\r\n"
           "Connection: close\r\n"
           "Content-Length: " + to_string(body.length()) + "\r\n"
           "\r\n" + body;
}

// Handle GET /api/health
string handleHealth()
{
    return createResponse(R"({"status": "ok", "message": "Server is running"})");
}

// Handle GET /api/status
string handleStatus()
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    json response;
    response["success"] = true;
    response["state"] = systemStateToJson(g_simulation->getState());
    response["isSafe"] = g_simulation->isSafe();
    response["safeSequence"] = g_simulation->getSafeSequence();
    
    return createResponse(response.dump());
}

// Handle POST /api/initialize
string handleInitialize(const string& body)
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    try
    {
        json jsonBody = json::parse(body);
        
        int processCount = jsonBody["processCount"];
        int resourceCount = jsonBody["resourceCount"];
        vector<vector<int>> allocation = jsonBody["allocation"];
        vector<vector<int>> maximum = jsonBody["maximum"];
        vector<int> available = jsonBody["available"];
        
        bool success = false;
        g_simulation = make_unique<Simulation>();
        if (jsonBody.contains("totalResources"))
        {
            vector<int> totalResources = jsonBody["totalResources"];
            success = g_simulation->initializeWithTotal(processCount, resourceCount, allocation, maximum, available, totalResources);
        }
        else
        {
            success = g_simulation->initialize(processCount, resourceCount, allocation, maximum, available);
        }

        if (!success)
        {
            g_simulation.reset();
            return createResponse(R"({"success": false, "error": "Invalid initialization parameters"})", 400);
        }
        
        json response;
        response["success"] = true;
        response["message"] = "Simulation initialized successfully";
        response["state"] = systemStateToJson(g_simulation->getState());
        
        return createResponse(response.dump());
    }
    catch (const exception& e)
    {
        return createResponse(R"({"success": false, "error": "Invalid JSON format"})", 400);
    }
}

// Handle POST /api/run
string handleRun()
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    bool success = g_simulation->run();
    
    json response;
    response["success"] = success;
    response["message"] = success ? "Simulation started successfully" : "Failed to start simulation";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    return createResponse(response.dump());
}

// Handle POST /api/step
string handleStep()
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    bool success = g_simulation->runOneStep();
    
    json response;
    response["success"] = success;
    response["message"] = success ? "Step executed successfully" : "Simulation finished or not running";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    return createResponse(response.dump());
}

// Handle POST /api/pause
string handlePause()
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    g_simulation->pause();
    
    json response;
    response["success"] = true;
    response["message"] = "Simulation paused";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    return createResponse(response.dump());
}

// Handle POST /api/resume
string handleResume()
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    g_simulation->resume();
    
    json response;
    response["success"] = true;
    response["message"] = "Simulation resumed";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    return createResponse(response.dump());
}

// Handle POST /api/fault
string handleFault(const string& body)
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    try
    {
        json jsonBody = json::parse(body);
        string typeStr = jsonBody["type"];
        int resourceID = jsonBody["resourceID"];
        int unitsLost = jsonBody["unitsLost"];
        
        FaultType type;
        if (typeStr == "RESOURCE_LOSS") type = RESOURCE_LOSS;
        else if (typeStr == "MEMORY_FRAGMENTATION") type = MEMORY_FRAGMENTATION;
        else if (typeStr == "HARDWARE_FAILURE") type = HARDWARE_FAILURE;
        else return createResponse(R"({"success": false, "error": "Invalid fault type"})", 400);
        
        FaultEvent faultEvent = g_simulation->injectFault(type, resourceID, unitsLost);
        bool success = !faultEvent.description.empty() &&
                       faultEvent.description != "Simulation is not running" &&
                       faultEvent.description != "Invalid resource ID" &&
                       faultEvent.description != "Invalid units lost";
        
        json response;
        response["success"] = success;
        response["message"] = success ? "Fault injected successfully" : faultEvent.description;
        response["fault"] = {
            {"id", faultEvent.id},
            {"type", faultTypeToString(faultEvent.type)},
            {"resourceID", faultEvent.resourceID},
            {"unitsLost", faultEvent.unitsLost},
            {"description", faultEvent.description},
            {"timestamp", faultEvent.timestamp}
        };
        response["state"] = systemStateToJson(g_simulation->getState());
        
        return createResponse(response.dump());
    }
    catch (const exception& e)
    {
        return createResponse(R"({"success": false, "error": "Invalid JSON format"})", 400);
    }
}

// Handle POST /api/recover
string handleRecover(const string& body)
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    try
    {
        json jsonBody = json::parse(body);
        string typeStr = jsonBody["type"];
        int processID = jsonBody.value("processID", -1);
        int resourceID = jsonBody.value("resourceID", -1);
        int units = jsonBody.value("units", 0);
        int fromProcess = jsonBody.value("fromProcess", -1);
        int toProcess = jsonBody.value("toProcess", -1);
        
        RecoveryType type;
        if (typeStr == "RESTORE_RESOURCE") type = RESTORE_RESOURCE;
        else if (typeStr == "SUSPEND_PROCESS") type = SUSPEND_PROCESS;
        else if (typeStr == "RESUME_PROCESS") type = RESUME_PROCESS;
        else if (typeStr == "TERMINATE_PROCESS") type = TERMINATE_PROCESS;
        else if (typeStr == "MANUAL_REALLOCATION") type = MANUAL_REALLOCATION;
        else return createResponse(R"({"success": false, "error": "Invalid recovery type"})", 400);
        
        RecoveryAction recoveryAction = g_simulation->recover(type, processID, resourceID, units, fromProcess, toProcess);
        
        json response;
        response["success"] = recoveryAction.success;
        response["message"] = recoveryAction.message;
        response["recovery"] = {
            {"id", recoveryAction.id},
            {"type", recoveryTypeToString(recoveryAction.type)},
            {"processID", recoveryAction.processID},
            {"resourceID", recoveryAction.resourceID},
            {"units", recoveryAction.units},
            {"success", recoveryAction.success},
            {"message", recoveryAction.message},
            {"timestamp", recoveryAction.timestamp}
        };
        response["state"] = systemStateToJson(g_simulation->getState());
        
        return createResponse(response.dump());
    }
    catch (const exception& e)
    {
        return createResponse(R"({"success": false, "error": "Invalid JSON format"})", 400);
    }
}

// Handle POST /api/reset
string handleReset()
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    g_simulation->reset();
    
    json response;
    response["success"] = true;
    response["message"] = "Simulation reset successfully";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    return createResponse(response.dump());
}

// Handle POST /api/speed
string handleSpeed(const string& body)
{
    lock_guard<mutex> lock(g_simulationMutex);
    
    if (!g_simulation)
    {
        return createResponse(R"({"success": false, "error": "Simulation not initialized"})", 400);
    }
    
    try
    {
        json jsonBody = json::parse(body);
        int speed = jsonBody["speed"];
        
        g_simulation->setSimulationSpeed(speed);
        
        json response;
        response["success"] = true;
        response["message"] = "Speed updated successfully";
        response["state"] = systemStateToJson(g_simulation->getState());
        
        return createResponse(response.dump());
    }
    catch (const exception& e)
    {
        return createResponse(R"({"success": false, "error": "Invalid JSON format"})", 400);
    }
}

// Parse HTTP request
struct HttpRequest
{
    string method;
    string path;
    string body;
};

HttpRequest parseRequest(const string& request)
{
    HttpRequest req;
    
    size_t methodEnd = request.find(' ');
    if (methodEnd == string::npos) return req;
    
    req.method = request.substr(0, methodEnd);
    
    size_t pathEnd = request.find(' ', methodEnd + 1);
    if (pathEnd == string::npos) return req;
    
    req.path = request.substr(methodEnd + 1, pathEnd - methodEnd - 1);
    
    // Find body (after empty line)
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart != string::npos)
    {
        req.body = request.substr(bodyStart + 4);
    }
    
    return req;
}

int getContentLength(const string& request)
{
    string headerEnd = "\r\n\r\n";
    size_t end = request.find(headerEnd);
    if (end == string::npos)
        return 0;

    string headers = request.substr(0, end);
    string key = "Content-Length:";
    size_t pos = headers.find(key);
    if (pos == string::npos)
        return 0;

    pos += key.length();
    while (pos < headers.length() && headers[pos] == ' ')
        pos++;

    size_t lineEnd = headers.find("\r\n", pos);
    string value = headers.substr(pos, lineEnd - pos);
    try
    {
        return stoi(value);
    }
    catch (...)
    {
        return 0;
    }
}

bool hasFullRequestBody(const string& request)
{
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart == string::npos)
        return false;

    int contentLength = getContentLength(request);
    size_t bodyLength = request.length() - (bodyStart + 4);
    return bodyLength >= static_cast<size_t>(contentLength);
}

// Read file content
string readFile(const string& path)
{
    FILE* file = fopen(path.c_str(), "rb");
    if (!file) 
    {
        cerr << "Failed to open file: " << path << endl;
        return "";
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    string content(size, '\0');
    fread(&content[0], 1, size, file);
    fclose(file);
    
    cerr << "Successfully read file: " << path << " (size: " << size << " bytes)" << endl;
    return content;
}

// Get current working directory
string getCurrentDir()
{
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of("\\/");
    return string(buffer).substr(0, pos);
}

// Handle client connection
void handleClient(SOCKET clientSocket)
{
    char buffer[8192];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0)
    {
        closesocket(clientSocket);
        return;
    }
    
    buffer[bytesReceived] = '\0';
    string request(buffer);

    while (!hasFullRequestBody(request))
    {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
            break;

        buffer[bytesReceived] = '\0';
        request.append(buffer, bytesReceived);
    }
    
    HttpRequest req = parseRequest(request);
    
    string response;
    
    // Get the correct frontend path
    string currentDir = getCurrentDir();
    string frontendPath = currentDir + "\\..\\frontend\\";
    
    // Handle CORS preflight
    if (req.method == "OPTIONS")
    {
        response = createResponse("", 200);
    }
    // API endpoints
    else if (req.path == "/api/health" && req.method == "GET")
    {
        response = handleHealth();
    }
    else if (req.path == "/api/status" && req.method == "GET")
    {
        response = handleStatus();
    }
    else if (req.path == "/api/initialize" && req.method == "POST")
    {
        response = handleInitialize(req.body);
    }
    else if (req.path == "/api/run" && req.method == "POST")
    {
        response = handleRun();
    }
    else if (req.path == "/api/step" && req.method == "POST")
    {
        response = handleStep();
    }
    else if (req.path == "/api/pause" && req.method == "POST")
    {
        response = handlePause();
    }
    else if (req.path == "/api/resume" && req.method == "POST")
    {
        response = handleResume();
    }
    else if (req.path == "/api/fault" && req.method == "POST")
    {
        response = handleFault(req.body);
    }
    else if (req.path == "/api/recover" && req.method == "POST")
    {
        response = handleRecover(req.body);
    }
    else if (req.path == "/api/reset" && req.method == "POST")
    {
        response = handleReset();
    }
    else if (req.path == "/api/speed" && req.method == "POST")
    {
        response = handleSpeed(req.body);
    }
    // Serve static files
    else if (req.path == "/" || req.path == "/index.html")
    {
        string content = readFile(frontendPath + "index.html");
        if (!content.empty())
        {
            response = createResponse(content, 200, "text/html; charset=utf-8");
        }
        else
        {
            response = createResponse(R"({"error": "File not found"})", 404);
        }
    }
    else if (req.path == "/style.css")
    {
        string content = readFile(frontendPath + "style.css");
        if (!content.empty())
        {
            response = createResponse(content, 200, "text/css; charset=utf-8");
        }
        else
        {
            response = createResponse(R"({"error": "File not found"})", 404);
        }
    }
    else if (req.path == "/script.js")
    {
        string content = readFile(frontendPath + "script.js");
        if (!content.empty())
        {
            response = createResponse(content, 200, "application/javascript; charset=utf-8");
        }
        else
        {
            response = createResponse(R"({"error": "File not found"})", 404);
        }
    }
    else
    {
        response = createResponse(R"({"error": "Not found"})", 404);
    }
    
    send(clientSocket, response.c_str(), response.length(), 0);
    closesocket(clientSocket);
}

// Start the HTTP server
void startServer(int port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed" << endl;
        return;
    }
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Failed to create socket" << endl;
        WSACleanup();
        return;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Failed to bind to port " << port << endl;
        cerr << "The port may be in use. Try a different port." << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
    
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        cerr << "Failed to listen on socket" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
    
    cout << "========================================" << endl;
    cout << "Dynamic Banker's Algorithm Simulator" << endl;
    cout << "========================================" << endl;
    cout << "Server starting on port " << port << "..." << endl;
    cout << "Frontend available at: http://localhost:" << port << endl;
    cout << "API endpoints:" << endl;
    cout << "  GET  /api/health" << endl;
    cout << "  GET  /api/status" << endl;
    cout << "  POST /api/initialize" << endl;
    cout << "  POST /api/run" << endl;
    cout << "  POST /api/step" << endl;
    cout << "  POST /api/pause" << endl;
    cout << "  POST /api/resume" << endl;
    cout << "  POST /api/fault" << endl;
    cout << "  POST /api/recover" << endl;
    cout << "  POST /api/reset" << endl;
    cout << "  POST /api/speed" << endl;
    cout << "========================================" << endl;
    
    while (true)
    {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        
        if (clientSocket == INVALID_SOCKET)
        {
            cerr << "Failed to accept client connection" << endl;
            continue;
        }
        
        thread(handleClient, clientSocket).detach();
    }
    
    closesocket(serverSocket);
    WSACleanup();
}
