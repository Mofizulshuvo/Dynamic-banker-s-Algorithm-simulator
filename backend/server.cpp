#include "server.h"
#include "simulation.h"
#include "external/httplib.h"
#include "external/json.hpp"
#include <iostream>
#include <memory>

using json = nlohmann::json;
using namespace std;

// Global simulation instance
unique_ptr<Simulation> g_simulation;

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

// Convert SystemState to JSON
json systemStateToJson(const SystemState& state)
{
    json j;
    
    j["processCount"] = state.processCount;
    j["resourceCount"] = state.resourceCount;
    j["allocation"] = state.allocation;
    j["maximum"] = state.maximum;
    j["need"] = state.need;
    j["available"] = state.available;
    j["totalResources"] = state.totalResources;
    j["safeSequence"] = state.safeSequence;
    j["simulationState"] = simulationStateToString(state.simulationState);
    j["currentStep"] = state.currentStep;
    j["simulationFinished"] = state.simulationFinished;
    j["simulationSpeed"] = state.simulationSpeed;
    
    // Convert processes
    json processesJson = json::array();
    for (const auto& process : state.processes)
    {
        json p;
        p["id"] = process.id;
        p["status"] = processStatusToString(process.status);
        p["progress"] = process.progress;
        processesJson.push_back(p);
    }
    j["processes"] = processesJson;
    
    // Convert fault history
    json faultHistoryJson = json::array();
    for (const auto& fault : state.faultHistory)
    {
        json f;
        f["id"] = fault.id;
        f["type"] = faultTypeToString(fault.type);
        f["resourceID"] = fault.resourceID;
        f["unitsLost"] = fault.unitsLost;
        f["description"] = fault.description;
        f["timestamp"] = fault.timestamp;
        faultHistoryJson.push_back(f);
    }
    j["faultHistory"] = faultHistoryJson;
    
    // Convert recovery history
    json recoveryHistoryJson = json::array();
    for (const auto& recovery : state.recoveryHistory)
    {
        json r;
        r["id"] = recovery.id;
        r["type"] = recoveryTypeToString(recovery.type);
        r["processID"] = recovery.processID;
        r["resourceID"] = recovery.resourceID;
        r["units"] = recovery.units;
        r["success"] = recovery.success;
        r["message"] = recovery.message;
        r["timestamp"] = recovery.timestamp;
        recoveryHistoryJson.push_back(r);
    }
    j["recoveryHistory"] = recoveryHistoryJson;
    
    // Convert timeline
    json timelineJson = json::array();
    for (const auto& event : state.timeline)
    {
        json t;
        t["id"] = event.id;
        t["event"] = event.event;
        t["processID"] = event.processID;
        t["available"] = event.available;
        t["safe"] = event.safe;
        t["timestamp"] = event.timestamp;
        timelineJson.push_back(t);
    }
    j["timeline"] = timelineJson;
    
    return j;
}

// Helper function to set CORS headers
void setCORSHeaders(httplib::Response& res)
{
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

// GET /status - Get current system status
void getStatus(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    json response;
    response["success"] = true;
    response["state"] = systemStateToJson(g_simulation->getState());
    response["isSafe"] = g_simulation->isSafe();
    response["safeSequence"] = g_simulation->getSafeSequence();
    
    res.set_content(response.dump(), "application/json");
}

// POST /initialize - Initialize the simulation
void initializeSimulation(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    try
    {
        json body = json::parse(req.body);
        
        int processCount = body["processCount"];
        int resourceCount = body["resourceCount"];
        vector<vector<int>> allocation = body["allocation"];
        vector<vector<int>> maximum = body["maximum"];
        vector<int> available = body["available"];
        
        g_simulation = make_unique<Simulation>();
        
        bool success;
        if (body.contains("totalResources"))
        {
            vector<int> totalResources = body["totalResources"];
            success = g_simulation->initializeWithTotal(processCount, resourceCount, allocation, maximum, available, totalResources);
        }
        else
        {
            success = g_simulation->initialize(processCount, resourceCount, allocation, maximum, available);
        }
        
        if (!success)
        {
            res.set_content(R"({"success": false, "error": "Invalid initialization parameters"})", "application/json");
            res.status = 400;
            return;
        }
        
        json response;
        response["success"] = true;
        response["message"] = "Simulation initialized successfully";
        response["state"] = systemStateToJson(g_simulation->getState());
        
        res.set_content(response.dump(), "application/json");
    }
    catch (const exception& e)
    {
        res.set_content(R"({"success": false, "error": "Invalid JSON format"})", "application/json");
        res.status = 400;
    }
}

// POST /run - Start the simulation
void runSimulation(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    bool success = g_simulation->run();
    
    json response;
    response["success"] = success;
    
    if (success)
    {
        response["message"] = "Simulation started successfully";
    }
    else
    {
        response["message"] = "System is in unsafe state, cannot start simulation";
    }
    
    response["state"] = systemStateToJson(g_simulation->getState());
    
    res.set_content(response.dump(), "application/json");
}

// POST /step - Execute one step of simulation
void stepSimulation(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    bool success = g_simulation->runOneStep();
    
    json response;
    response["success"] = success;
    response["message"] = success ? "Step executed successfully" : "Simulation finished or not running";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    res.set_content(response.dump(), "application/json");
}

// POST /pause - Pause the simulation
void pauseSimulation(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    g_simulation->pause();
    
    json response;
    response["success"] = true;
    response["message"] = "Simulation paused";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    res.set_content(response.dump(), "application/json");
}

// POST /resume - Resume the simulation
void resumeSimulation(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    g_simulation->resume();
    
    json response;
    response["success"] = true;
    response["message"] = "Simulation resumed";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    res.set_content(response.dump(), "application/json");
}

// POST /fault - Inject a fault
void injectFault(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    try
    {
        json body = json::parse(req.body);
        
        string typeStr = body["type"];
        int resourceID = body["resourceID"];
        int unitsLost = body["unitsLost"];
        
        FaultType type;
        if (typeStr == "RESOURCE_LOSS")
            type = RESOURCE_LOSS;
        else if (typeStr == "MEMORY_FRAGMENTATION")
            type = MEMORY_FRAGMENTATION;
        else if (typeStr == "HARDWARE_FAILURE")
            type = HARDWARE_FAILURE;
        else
        {
            res.set_content(R"({"success": false, "error": "Invalid fault type"})", "application/json");
            res.status = 400;
            return;
        }
        
        FaultEvent fault = g_simulation->injectFault(type, resourceID, unitsLost);
        
        json response;
        response["success"] = true;
        response["message"] = "Fault injected successfully";
        response["fault"] = {
            {"id", fault.id},
            {"type", faultTypeToString(fault.type)},
            {"resourceID", fault.resourceID},
            {"unitsLost", fault.unitsLost},
            {"description", fault.description},
            {"timestamp", fault.timestamp}
        };
        response["state"] = systemStateToJson(g_simulation->getState());
        
        res.set_content(response.dump(), "application/json");
    }
    catch (const exception& e)
    {
        res.set_content(R"({"success": false, "error": "Invalid JSON format"})", "application/json");
        res.status = 400;
    }
}

// POST /recover - Apply recovery
void applyRecovery(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    try
    {
        json body = json::parse(req.body);
        
        string typeStr = body["type"];
        int processID = body.value("processID", -1);
        int resourceID = body.value("resourceID", -1);
        int units = body.value("units", 0);
        int fromProcess = body.value("fromProcess", -1);
        int toProcess = body.value("toProcess", -1);
        
        RecoveryType type;
        if (typeStr == "RESTORE_RESOURCE")
            type = RESTORE_RESOURCE;
        else if (typeStr == "SUSPEND_PROCESS")
            type = SUSPEND_PROCESS;
        else if (typeStr == "RESUME_PROCESS")
            type = RESUME_PROCESS;
        else if (typeStr == "TERMINATE_PROCESS")
            type = TERMINATE_PROCESS;
        else if (typeStr == "MANUAL_REALLOCATION")
            type = MANUAL_REALLOCATION;
        else
        {
            res.set_content(R"({"success": false, "error": "Invalid recovery type"})", "application/json");
            res.status = 400;
            return;
        }
        
        RecoveryAction action = g_simulation->recover(type, processID, resourceID, units, fromProcess, toProcess);
        
        json response;
        response["success"] = action.success;
        response["message"] = action.message;
        response["recovery"] = {
            {"id", action.id},
            {"type", recoveryTypeToString(action.type)},
            {"processID", action.processID},
            {"resourceID", action.resourceID},
            {"units", action.units},
            {"success", action.success},
            {"message", action.message},
            {"timestamp", action.timestamp}
        };
        response["state"] = systemStateToJson(g_simulation->getState());
        
        res.set_content(response.dump(), "application/json");
    }
    catch (const exception& e)
    {
        res.set_content(R"({"success": false, "error": "Invalid JSON format"})", "application/json");
        res.status = 400;
    }
}

// POST /reset - Reset the simulation
void resetSimulation(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    g_simulation->reset();
    
    json response;
    response["success"] = true;
    response["message"] = "Simulation reset successfully";
    response["state"] = systemStateToJson(g_simulation->getState());
    
    res.set_content(response.dump(), "application/json");
}

// POST /speed - Set simulation speed
void setSpeed(const httplib::Request& req, httplib::Response& res)
{
    setCORSHeaders(res);
    
    if (!g_simulation)
    {
        res.set_content(R"({"success": false, "error": "Simulation not initialized"})", "application/json");
        res.status = 400;
        return;
    }
    
    try
    {
        json body = json::parse(req.body);
        int speed = body["speed"];
        
        g_simulation->setSimulationSpeed(speed);
        
        json response;
        response["success"] = true;
        response["message"] = "Simulation speed updated";
        response["speed"] = g_simulation->getSimulationSpeed();
        
        res.set_content(response.dump(), "application/json");
    }
    catch (const exception& e)
    {
        res.set_content(R"({"success": false, "error": "Invalid JSON format"})", "application/json");
        res.status = 400;
    }
}

// Start the HTTP server
void startServer(int port)
{
    httplib::Server svr;
    
    // Handle OPTIONS requests for CORS
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCORSHeaders(res);
        return;
    });
    
    // API endpoints
    svr.Get("/api/status", getStatus);
    svr.Post("/api/initialize", initializeSimulation);
    svr.Post("/api/run", runSimulation);
    svr.Post("/api/step", stepSimulation);
    svr.Post("/api/pause", pauseSimulation);
    svr.Post("/api/resume", resumeSimulation);
    svr.Post("/api/fault", injectFault);
    svr.Post("/api/recover", applyRecovery);
    svr.Post("/api/reset", resetSimulation);
    svr.Post("/api/speed", setSpeed);
    
    // Serve static files
    svr.set_mount_point("/", "../frontend");
    
    cout << "========================================" << endl;
    cout << "Dynamic Banker's Algorithm Simulator" << endl;
    cout << "========================================" << endl;
    cout << "Server starting on port " << port << "..." << endl;
    cout << "Frontend available at: http://localhost:" << port << endl;
    cout << "API endpoints:" << endl;
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
    
    svr.listen("0.0.0.0", port);
}
