#include "server.h"

#include "external/httplib.h"
#include "external/json.hpp"
#include "simulation.h"

#include <iostream>
#include <memory>
#include <mutex>

using json = nlohmann::json;
using namespace std;

namespace
{
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

    j["processes"] = json::array();
    for (const auto& process : state.processes)
    {
        j["processes"].push_back({
            {"id", process.id},
            {"status", processStatusToString(process.status)},
            {"progress", process.progress}
        });
    }

    j["faultHistory"] = json::array();
    for (const auto& fault : state.faultHistory)
    {
        j["faultHistory"].push_back({
            {"id", fault.id},
            {"type", faultTypeToString(fault.type)},
            {"resourceID", fault.resourceID},
            {"unitsLost", fault.unitsLost},
            {"description", fault.description},
            {"timestamp", fault.timestamp}
        });
    }

    j["recoveryHistory"] = json::array();
    for (const auto& recovery : state.recoveryHistory)
    {
        j["recoveryHistory"].push_back({
            {"id", recovery.id},
            {"type", recoveryTypeToString(recovery.type)},
            {"processID", recovery.processID},
            {"resourceID", recovery.resourceID},
            {"units", recovery.units},
            {"success", recovery.success},
            {"message", recovery.message},
            {"timestamp", recovery.timestamp}
        });
    }

    j["timeline"] = json::array();
    for (const auto& event : state.timeline)
    {
        j["timeline"].push_back({
            {"id", event.id},
            {"event", event.event},
            {"processID", event.processID},
            {"available", event.available},
            {"safe", event.safe},
            {"timestamp", event.timestamp}
        });
    }

    return j;
}

void setCors(httplib::Response& res)
{
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

void sendJson(httplib::Response& res, const json& body, int status = 200)
{
    setCors(res);
    res.status = status;
    res.set_content(body.dump(), "application/json; charset=utf-8");
}

bool parseFaultType(const string& value, FaultType& type)
{
    if (value == "RESOURCE_LOSS")
        type = RESOURCE_LOSS;
    else if (value == "MEMORY_FRAGMENTATION")
        type = MEMORY_FRAGMENTATION;
    else if (value == "HARDWARE_FAILURE")
        type = HARDWARE_FAILURE;
    else
        return false;

    return true;
}

bool parseRecoveryType(const string& value, RecoveryType& type)
{
    if (value == "RESTORE_RESOURCE")
        type = RESTORE_RESOURCE;
    else if (value == "SUSPEND_PROCESS")
        type = SUSPEND_PROCESS;
    else if (value == "RESUME_PROCESS")
        type = RESUME_PROCESS;
    else if (value == "TERMINATE_PROCESS")
        type = TERMINATE_PROCESS;
    else if (value == "MANUAL_REALLOCATION")
        type = MANUAL_REALLOCATION;
    else
        return false;

    return true;
}
}

void startServer(int port)
{
    httplib::Server server;
    mutex simulationMutex;
    unique_ptr<Simulation> simulation;

    server.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        setCors(res);
    });

    auto healthHandler = [](const httplib::Request&, httplib::Response& res) {
        sendJson(res, {{"status", "ok"}, {"message", "Server is running"}});
    };

    auto statusHandler = [&](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        sendJson(res, {
            {"success", true},
            {"state", systemStateToJson(simulation->getState())},
            {"isSafe", simulation->isSafe()},
            {"safeSequence", simulation->getSafeSequence()}
        });
    };

    auto initializeHandler = [&](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        try
        {
            json body = json::parse(req.body);
            int processCount = body.at("processCount");
            int resourceCount = body.at("resourceCount");
            vector<vector<int>> allocation = body.at("allocation");
            vector<vector<int>> maximum = body.at("maximum");
            vector<int> available = body.at("available");

            auto nextSimulation = make_unique<Simulation>();
            bool success = false;
            if (body.contains("totalResources"))
            {
                vector<int> totalResources = body.at("totalResources");
                success = nextSimulation->initializeWithTotal(
                    processCount,
                    resourceCount,
                    allocation,
                    maximum,
                    available,
                    totalResources
                );
            }
            else
            {
                success = nextSimulation->initialize(processCount, resourceCount, allocation, maximum, available);
            }

            if (!success)
            {
                sendJson(res, {{"success", false}, {"error", "Invalid initialization parameters"}}, 400);
                return;
            }

            simulation = move(nextSimulation);
            sendJson(res, {
                {"success", true},
                {"message", "Simulation initialized successfully"},
                {"state", systemStateToJson(simulation->getState())}
            });
        }
        catch (const exception& e)
        {
            sendJson(res, {{"success", false}, {"error", string("Invalid request: ") + e.what()}}, 400);
        }
    };

    auto runHandler = [&](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        bool success = simulation->run();
        sendJson(res, {
            {"success", success},
            {"message", success ? "Simulation started successfully" : "System is unsafe; no safe sequence exists"},
            {"state", systemStateToJson(simulation->getState())}
        });
    };

    auto stepHandler = [&](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        bool success = simulation->runOneStep();
        sendJson(res, {
            {"success", success},
            {"message", success ? "Step executed successfully" : "Simulation finished or not running"},
            {"state", systemStateToJson(simulation->getState())}
        });
    };

    auto pauseHandler = [&](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        simulation->pause();
        sendJson(res, {
            {"success", true},
            {"message", "Simulation paused"},
            {"state", systemStateToJson(simulation->getState())}
        });
    };

    auto resumeHandler = [&](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        simulation->resume();
        sendJson(res, {
            {"success", true},
            {"message", "Simulation resumed"},
            {"state", systemStateToJson(simulation->getState())}
        });
    };

    auto faultHandler = [&](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        try
        {
            json body = json::parse(req.body);
            FaultType type;
            if (!parseFaultType(body.at("type"), type))
            {
                sendJson(res, {{"success", false}, {"error", "Invalid fault type"}}, 400);
                return;
            }

            FaultEvent fault = simulation->injectFault(
                type,
                body.at("resourceID"),
                body.at("unitsLost")
            );

            bool success = fault.id >= 0;
            sendJson(res, {
                {"success", success},
                {"message", success ? "Fault injected successfully" : fault.description},
                {"fault", {
                    {"id", fault.id},
                    {"type", faultTypeToString(fault.type)},
                    {"resourceID", fault.resourceID},
                    {"unitsLost", fault.unitsLost},
                    {"description", fault.description},
                    {"timestamp", fault.timestamp}
                }},
                {"state", systemStateToJson(simulation->getState())}
            });
        }
        catch (const exception& e)
        {
            sendJson(res, {{"success", false}, {"error", string("Invalid request: ") + e.what()}}, 400);
        }
    };

    auto recoverHandler = [&](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        try
        {
            json body = json::parse(req.body);
            RecoveryType type;
            if (!parseRecoveryType(body.at("type"), type))
            {
                sendJson(res, {{"success", false}, {"error", "Invalid recovery type"}}, 400);
                return;
            }

            RecoveryAction action = simulation->recover(
                type,
                body.value("processID", -1),
                body.value("resourceID", -1),
                body.value("units", 0),
                body.value("fromProcess", -1),
                body.value("toProcess", -1)
            );

            sendJson(res, {
                {"success", action.success},
                {"message", action.message},
                {"recovery", {
                    {"id", action.id},
                    {"type", recoveryTypeToString(action.type)},
                    {"processID", action.processID},
                    {"resourceID", action.resourceID},
                    {"units", action.units},
                    {"success", action.success},
                    {"message", action.message},
                    {"timestamp", action.timestamp}
                }},
                {"state", systemStateToJson(simulation->getState())}
            });
        }
        catch (const exception& e)
        {
            sendJson(res, {{"success", false}, {"error", string("Invalid request: ") + e.what()}}, 400);
        }
    };

    auto resetHandler = [&](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        simulation->reset();
        sendJson(res, {
            {"success", true},
            {"message", "Simulation reset successfully"},
            {"state", systemStateToJson(simulation->getState())}
        });
    };

    auto speedHandler = [&](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(simulationMutex);
        if (!simulation)
        {
            sendJson(res, {{"success", false}, {"error", "Simulation not initialized"}}, 400);
            return;
        }

        try
        {
            json body = json::parse(req.body);
            simulation->setSimulationSpeed(body.at("speed"));
            sendJson(res, {
                {"success", true},
                {"message", "Simulation speed updated"},
                {"speed", simulation->getSimulationSpeed()},
                {"state", systemStateToJson(simulation->getState())}
            });
        }
        catch (const exception& e)
        {
            sendJson(res, {{"success", false}, {"error", string("Invalid request: ") + e.what()}}, 400);
        }
    };

    server.Get("/api/health", healthHandler);
    server.Get("/health", healthHandler);
    server.Get("/api/status", statusHandler);
    server.Get("/status", statusHandler);
    server.Post("/api/initialize", initializeHandler);
    server.Post("/initialize", initializeHandler);
    server.Post("/api/run", runHandler);
    server.Post("/run", runHandler);
    server.Post("/api/step", stepHandler);
    server.Post("/step", stepHandler);
    server.Post("/api/pause", pauseHandler);
    server.Post("/pause", pauseHandler);
    server.Post("/api/resume", resumeHandler);
    server.Post("/resume", resumeHandler);
    server.Post("/api/fault", faultHandler);
    server.Post("/fault", faultHandler);
    server.Post("/api/recover", recoverHandler);
    server.Post("/recover", recoverHandler);
    server.Post("/api/reset", resetHandler);
    server.Post("/reset", resetHandler);
    server.Post("/api/speed", speedHandler);
    server.Post("/speed", speedHandler);

    server.set_mount_point("/", "./frontend");
    server.set_file_extension_and_mimetype_mapping(".html", "text/html; charset=utf-8");
    server.set_file_extension_and_mimetype_mapping(".css", "text/css; charset=utf-8");
    server.set_file_extension_and_mimetype_mapping(".js", "application/javascript; charset=utf-8");

    cout << "========================================" << endl;
    cout << "Dynamic Banker's Algorithm Simulator" << endl;
    cout << "========================================" << endl;
    cout << "Frontend: http://localhost:" << port << endl;
    cout << "API:      http://localhost:" << port << "/api/status" << endl;
    cout << "========================================" << endl;

    if (!server.listen("0.0.0.0", port))
    {
        cerr << "Failed to start server on port " << port << endl;
        cerr << "Close any old server.exe process and try again." << endl;
    }
}
