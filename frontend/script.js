const API_BASE = "/api";

const defaultState = {
    processes: ["P0", "P1", "P2", "P3", "P4"],
    resources: ["A", "B", "C"],
    allocation: [
        [0, 1, 0],
        [2, 0, 0],
        [3, 0, 2],
        [2, 1, 1],
        [0, 0, 2]
    ],
    max: [
        [7, 5, 3],
        [3, 2, 2],
        [9, 0, 2],
        [2, 2, 2],
        [4, 3, 3]
    ],
    need: [],
    available: [3, 3, 2],
    totalResources: [10, 5, 7],
    serverProcesses: [],
    safeSequence: [],
    timeline: [],
    simulationState: "IDLE",
    currentStep: 0,
    initialized: false,
    simulationSpeed: 1000
};

let state = structuredClone(defaultState);
let runTimer = null;
let serverOnline = false;
let matrixView = "all";

document.addEventListener("DOMContentLoaded", () => {
    setupThemeToggle();
    renderAll();
    checkServerStatus();
});

async function checkServerStatus() {
    try {
        const response = await fetch(`${API_BASE}/health`);
        serverOnline = response.ok;
        updateServerBadge();

        if (!serverOnline) return false;

        const status = await fetch(`${API_BASE}/status`);
        if (status.ok) {
            const data = await status.json();
            if (data.success && data.state) {
                state.initialized = data.state.simulationState !== "IDLE";
                updateUIFromState(data.state);
                enableControls(state.initialized);
            }
        }
        return true;
    } catch {
        serverOnline = false;
        updateServerBadge();
        showToast("Backend server is not running. Start it with run.bat.", "warning");
        return false;
    }
}

function updateServerBadge() {
    const badge = document.getElementById("serverBadge");
    badge.textContent = serverOnline ? "Online" : "Offline";
    badge.style.color = serverOnline ? "var(--success)" : "var(--danger)";
}

async function apiCall(endpoint, method = "GET", body = null, quiet = false) {
    try {
        const options = {
            method,
            headers: { "Content-Type": "application/json" }
        };
        if (body) options.body = JSON.stringify(body);

        const response = await fetch(`${API_BASE}${endpoint}`, options);
        const text = await response.text();
        const data = text ? JSON.parse(text) : {};

        if (!response.ok) {
            throw new Error(data.error || data.message || `HTTP ${response.status}`);
        }

        serverOnline = true;
        updateServerBadge();
        return data;
    } catch (error) {
        serverOnline = false;
        updateServerBadge();
        if (!quiet) showToast(error.message || "API request failed", "error");
        return { success: false, error: error.message };
    }
}

async function initializeSystem() {
    const validation = validateConfiguration();
    if (!validation.ok) {
        showToast(validation.message, "error");
        return;
    }

    showLoading(true);
    const result = await apiCall("/initialize", "POST", {
        processCount: state.processes.length,
        resourceCount: state.resources.length,
        allocation: state.allocation,
        maximum: state.max,
        available: state.available,
        totalResources: state.totalResources
    });
    showLoading(false);

    if (result.success) {
        stopAutoRun();
        state.initialized = true;
        updateUIFromState(result.state);
        enableControls(true);
        showToast("System initialized.", "success");
    }
}

function loadSampleConfiguration() {
    stopAutoRun();
    state = structuredClone(defaultState);
    renderAll();
    enableControls(false);
    showToast("Classic sample configuration loaded.", "success");
}

function loadDeadlockConfiguration() {
    stopAutoRun();
    state = {
        processes: ["P0", "P1"],
        resources: ["A", "B"],
        allocation: [
            [1, 0],
            [0, 1]
        ],
        max: [
            [1, 1],
            [1, 1]
        ],
        need: [],
        available: [0, 0],
        totalResources: [1, 1],
        serverProcesses: [],
        safeSequence: [],
        timeline: [],
        simulationState: "IDLE",
        currentStep: 0,
        initialized: false,
        simulationSpeed: state.simulationSpeed || 1000
    };
    renderAll();
    enableControls(false);
    showToast("Deadlock preset loaded. Initialize, then Run.", "warning");
}

async function runSimulation() {
    if (!state.initialized) {
        showToast("Initialize the system first.", "warning");
        return;
    }

    showLoading(true);
    const result = await apiCall("/run", "POST");
    showLoading(false);

    if (result.success) {
        updateUIFromState(result.state);
        startAutoRun();
        showToast("Simulation running.", "success");
    } else {
        updateUIFromState(result.state);
        showToast(result.message || "System is unsafe.", "error");
    }
}

async function stepSimulation(manual = true) {
    if (!state.initialized) {
        showToast("Initialize the system first.", "warning");
        return false;
    }

    const result = await apiCall("/step", "POST", null, !manual);
    if (result.state) updateUIFromState(result.state);

    if (!result.success) {
        stopAutoRun();
        if (manual) showToast(result.message || "No step was executed.", "info");
        return false;
    }

    if (manual) showToast("Step executed.", "success");
    return true;
}

async function pauseSimulation() {
    stopAutoRun();
    const result = await apiCall("/pause", "POST");
    if (result.success) {
        updateUIFromState(result.state);
        showToast("Simulation paused.", "info");
    }
}

async function resumeSimulation() {
    const result = await apiCall("/resume", "POST");
    if (result.success) {
        updateUIFromState(result.state);
        startAutoRun();
        showToast("Simulation resumed.", "success");
    }
}

async function resetSystem() {
    stopAutoRun();

    if (!state.initialized) {
        state = structuredClone(defaultState);
        renderAll();
        enableControls(false);
        showToast("Configuration restored.", "success");
        return;
    }

    const result = await apiCall("/reset", "POST");
    if (result.success) {
        updateUIFromState(result.state);
        showToast("Simulation reset.", "success");
    } else {
        state = structuredClone(defaultState);
        renderAll();
        enableControls(false);
    }
}

function newConfiguration() {
    stopAutoRun();
    state.initialized = false;
    state.simulationState = "IDLE";
    state.currentStep = 0;
    state.serverProcesses = [];
    state.safeSequence = [];
    state.timeline = [];
    state.need = [];
    renderAll();
    enableControls(false);
    showToast("Configuration is editable again.", "info");
}

function startAutoRun() {
    stopAutoRun();
    runTimer = setInterval(async () => {
        if (state.simulationState !== "RUNNING") {
            stopAutoRun();
            return;
        }
        await stepSimulation(false);
    }, state.simulationSpeed);
    updateControlState();
}

function stopAutoRun() {
    if (runTimer) clearInterval(runTimer);
    runTimer = null;
    updateControlState();
}

async function updateSpeed(speed) {
    state.simulationSpeed = Number(speed);
    document.getElementById("speedValue").textContent = `${state.simulationSpeed} ms`;

    if (state.initialized) {
        await apiCall("/speed", "POST", { speed: state.simulationSpeed }, true);
    }

    if (runTimer) startAutoRun();
}

function togglePanel(panelId, force) {
    const panel = document.getElementById(panelId);
    const shouldShow = typeof force === "boolean" ? force : panel.hidden;
    panel.hidden = !shouldShow;

    if (panelId === "faultPanel" && shouldShow) renderFaultOptions();
    if (panelId === "recoveryPanel" && shouldShow) renderRecoveryOptions();
}

function renderFaultOptions() {
    const select = document.getElementById("faultResourceID");
    select.innerHTML = state.resources.map((name, index) => `<option value="${index}">${name}</option>`).join("");
}

async function injectFault() {
    const type = document.getElementById("faultType").value;
    const resourceID = Number(document.getElementById("faultResourceID").value);
    const unitsLost = Number(document.getElementById("faultUnitsLost").value);

    if (!Number.isInteger(unitsLost) || unitsLost <= 0) {
        showToast("Units lost must be a positive number.", "error");
        return;
    }

    showLoading(true);
    const result = await apiCall("/fault", "POST", { type, resourceID, unitsLost });
    showLoading(false);

    if (result.success) {
        stopAutoRun();
        updateUIFromState(result.state);
        togglePanel("faultPanel", false);
        if (result.state?.simulationState === "UNSAFE") {
            togglePanel("recoveryPanel", true);
        }
        showToast(result.fault?.description || "Fault injected.", "warning");
    } else {
        if (result.state) updateUIFromState(result.state);
        showToast(result.message || result.fault?.description || "Fault injection failed.", "error");
    }
}

function renderRecoveryOptions() {
    const resourceOptions = state.resources.map((name, index) => `<option value="${index}">${name}</option>`).join("");
    const processOptions = state.processes.map((name, index) => `<option value="${index}">${name}</option>`).join("");

    document.getElementById("recoveryOptions").innerHTML = `
        <div class="recovery-card">
            <h3>Restore Resource</h3>
            <div class="form-grid">
                <label>Resource<select id="recoveryResourceID">${resourceOptions}</select></label>
                <label>Units<input type="number" id="recoveryUnits" value="1" min="1"></label>
                <button class="btn btn-success" type="button" onclick="applyRecovery('RESTORE_RESOURCE')">Apply</button>
            </div>
        </div>
        <div class="recovery-card">
            <h3>Suspend Process</h3>
            <div class="form-grid">
                <label>Process<select id="suspendProcessID">${processOptions}</select></label>
                <button class="btn btn-warning" type="button" onclick="applyRecovery('SUSPEND_PROCESS')">Apply</button>
            </div>
        </div>
        <div class="recovery-card">
            <h3>Resume Process</h3>
            <div class="form-grid">
                <label>Process<select id="resumeProcessID">${processOptions}</select></label>
                <button class="btn btn-info" type="button" onclick="applyRecovery('RESUME_PROCESS')">Apply</button>
            </div>
        </div>
        <div class="recovery-card">
            <h3>Terminate Process</h3>
            <div class="form-grid">
                <label>Process<select id="terminateProcessID">${processOptions}</select></label>
                <button class="btn btn-danger" type="button" onclick="applyRecovery('TERMINATE_PROCESS')">Apply</button>
            </div>
        </div>
        <div class="recovery-card">
            <h3>Manual Reallocation</h3>
            <div class="form-grid">
                <label>From<select id="fromProcessID">${processOptions}</select></label>
                <label>To<select id="toProcessID">${processOptions}</select></label>
                <label>Resource<select id="manualResourceID">${resourceOptions}</select></label>
                <label>Units<input type="number" id="manualUnits" value="1" min="1"></label>
                <button class="btn btn-outline" type="button" onclick="applyRecovery('MANUAL_REALLOCATION')">Apply</button>
            </div>
        </div>
    `;
}

async function applyRecovery(type) {
    const body = { type };

    if (type === "RESTORE_RESOURCE") {
        body.resourceID = Number(document.getElementById("recoveryResourceID").value);
        body.units = Number(document.getElementById("recoveryUnits").value);
    }
    if (type === "SUSPEND_PROCESS") body.processID = Number(document.getElementById("suspendProcessID").value);
    if (type === "RESUME_PROCESS") body.processID = Number(document.getElementById("resumeProcessID").value);
    if (type === "TERMINATE_PROCESS") body.processID = Number(document.getElementById("terminateProcessID").value);
    if (type === "MANUAL_REALLOCATION") {
        body.fromProcess = Number(document.getElementById("fromProcessID").value);
        body.toProcess = Number(document.getElementById("toProcessID").value);
        body.resourceID = Number(document.getElementById("manualResourceID").value);
        body.units = Number(document.getElementById("manualUnits").value);
    }

    showLoading(true);
    const result = await apiCall("/recover", "POST", body);
    showLoading(false);

    if (result.success) {
        updateUIFromState(result.state);
        togglePanel("recoveryPanel", false);
        if (result.state?.simulationState === "RUNNING") {
            startAutoRun();
        }
        showToast(result.recovery?.message || "Recovery applied.", "success");
    } else {
        if (result.state) updateUIFromState(result.state);
        showToast(result.recovery?.message || result.message || "Recovery failed.", "error");
    }
}

function updateUIFromState(serverState) {
    state.allocation = normalizeMatrix(serverState.allocation, state.processes.length, state.resources.length);
    state.max = normalizeMatrix(serverState.maximum, state.processes.length, state.resources.length);
    state.need = normalizeMatrix(serverState.need, state.processes.length, state.resources.length);
    state.available = normalizeVector(serverState.available, state.resources.length);
    state.totalResources = normalizeVector(serverState.totalResources, state.resources.length, state.available);
    state.safeSequence = serverState.safeSequence || [];
    state.timeline = serverState.timeline || [];
    state.serverProcesses = serverState.processes || [];
    state.simulationState = serverState.simulationState || "IDLE";
    state.currentStep = serverState.currentStep || 0;
    state.initialized = state.simulationState !== "IDLE";

    renderAll();
    updateControlState();
}

function renderAll() {
    renderSummary();
    renderIncidentBanner();
    renderResourceBars();
    renderValidation();
    renderResourceTable();
    renderAllocation();
    renderMax();
    renderNeed();
    renderProcessCards();
    renderSafeSequence();
    renderTimeline();
    applyMatrixView();
}

function renderSummary() {
    const stateBadge = document.getElementById("stateBadge");
    stateBadge.textContent = formatState(state.simulationState);
    stateBadge.style.color = getStateColor(state.simulationState);
    document.getElementById("processCountLabel").textContent = String(state.processes.length);
    document.getElementById("resourceCountLabel").textContent = String(state.resources.length);
    document.getElementById("stepLabel").textContent = `${state.currentStep} / ${state.safeSequence.length}`;
    document.getElementById("statusMessage").textContent = getStatusMessage();
    document.getElementById("sequenceHint").textContent = state.safeSequence.length ? "Calculated by safety check" : "No sequence";
    document.getElementById("speedValue").textContent = `${state.simulationSpeed} ms`;
    renderSafetyScore();
}

function renderSafetyScore() {
    const score = document.getElementById("safetyScore");
    const fill = document.getElementById("safetyFill");
    const validation = getValidationMessages();
    const isSafe = state.simulationState === "RUNNING" || state.simulationState === "INITIALIZED" || state.simulationState === "COMPLETED";
    const hasSequence = state.safeSequence.length > 0;
    const hasErrors = validation.some(item => item.type === "error");
    let label = "Pending";
    let percent = 34;
    let className = "";

    if (hasErrors) {
        label = "Invalid";
        percent = 18;
        className = "danger";
    } else if (state.simulationState === "UNSAFE") {
        label = "Unsafe";
        percent = 28;
        className = "danger";
    } else if (hasSequence || isSafe) {
        label = "Safe";
        percent = 100;
    } else if (state.initialized) {
        label = "Ready";
        percent = 72;
    }

    score.textContent = label;
    fill.style.width = `${percent}%`;
    fill.className = `bar-fill ${className}`;
}

function renderIncidentBanner() {
    const banner = document.getElementById("incidentBanner");
    const title = document.getElementById("incidentTitle");
    const message = document.getElementById("incidentMessage");
    const eyebrow = document.getElementById("incidentEyebrow");
    const resumeButton = document.getElementById("incidentResumeBtn");

    banner.hidden = true;
    banner.classList.remove("unsafe");
    resumeButton.hidden = false;

    if (state.simulationState === "UNSAFE") {
        banner.hidden = false;
        banner.classList.add("unsafe");
        eyebrow.textContent = "Unsafe State";
        title.textContent = "No safe sequence exists";
        message.textContent = "The system is at deadlock risk. Apply a recovery strategy before execution can continue.";
        resumeButton.hidden = true;
        return;
    }

    if (state.simulationState === "PAUSED") {
        const lastFault = [...state.timeline].reverse().find(item => item.event?.toLowerCase().includes("fault"));
        if (!lastFault) return;

        banner.hidden = false;
        eyebrow.textContent = "Fault Review";
        title.textContent = "Runtime fault paused the simulation";
        message.textContent = "Resources changed during execution. Review the recalculated state, recover if needed, or resume.";
    }
}

function renderResourceBars() {
    const container = document.getElementById("resourceBars");
    container.innerHTML = state.resources.map((resource, index) => {
        const total = Math.max(1, state.totalResources[index] || state.available[index] || 1);
        const available = state.available[index] || 0;
        const percent = clamp((available / total) * 100, 0, 100);
        return `
            <div class="resource-item">
                <div class="resource-head">
                    <span class="resource-name">Resource ${resource}</span>
                    <span class="resource-value">${available} / ${total}</span>
                </div>
                <div class="bar"><div class="bar-fill" style="width:${percent}%"></div></div>
                <div class="resource-meta">
                    <span class="muted">Allocated ${Math.max(0, total - available)}</span>
                    <span class="muted">${Math.round(percent)}% free</span>
                </div>
            </div>
        `;
    }).join("");
}

function renderValidation() {
    const messages = getValidationMessages();
    const summary = document.getElementById("validationSummary");
    const list = document.getElementById("validationList");
    const hasErrors = messages.some(item => item.type === "error");
    const warnings = messages.filter(item => item.type === "warning").length;

    summary.textContent = hasErrors ? "Needs attention" : warnings ? "Review suggested" : "Ready";
    summary.style.color = hasErrors ? "var(--danger)" : warnings ? "var(--warning)" : "var(--success)";

    list.innerHTML = messages.map(item => `
        <div class="validation-item ${item.type}">
            <div class="validation-dot"></div>
            <div>
                <strong>${item.title}</strong>
                <p class="muted">${item.message}</p>
            </div>
        </div>
    `).join("");
}

function renderResourceTable() {
    const table = document.getElementById("resourceTable");
    table.innerHTML = `
        <thead><tr><th>Metric</th>${state.resources.map(r => `<th>${r}</th>`).join("")}</tr></thead>
        <tbody>
            <tr>
                <td>Available</td>
                ${state.resources.map((_, j) => cellInput(state.available[j], `updateAvailable(${j}, this.value)`)).join("")}
            </tr>
            <tr>
                <td>Total</td>
                ${state.resources.map((_, j) => cellInput(state.totalResources[j], `updateTotal(${j}, this.value)`)).join("")}
            </tr>
        </tbody>
    `;
}

function renderAllocation() {
    renderMatrix("allocationTable", state.allocation, "updateAllocation");
}

function renderMax() {
    renderMatrix("maxTable", state.max, "updateMax");
}

function renderMatrix(tableId, matrix, handler) {
    const table = document.getElementById(tableId);
    table.innerHTML = `
        <thead><tr><th>Process</th>${state.resources.map(r => `<th>${r}</th>`).join("")}</tr></thead>
        <tbody>
            ${state.processes.map((process, i) => `
                <tr>
                    <td>${process}</td>
                    ${state.resources.map((_, j) => cellInput(matrix[i][j], `${handler}(${i}, ${j}, this.value)`)).join("")}
                </tr>
            `).join("")}
        </tbody>
    `;
}

function renderNeed() {
    const table = document.getElementById("needTable");
    table.innerHTML = `
        <thead><tr><th>Process</th>${state.resources.map(r => `<th>${r}</th>`).join("")}</tr></thead>
        <tbody>
            ${state.processes.map((process, i) => `
                <tr>
                    <td>${process}</td>
                    ${state.resources.map((_, j) => `<td>${getNeed(i, j)}</td>`).join("")}
                </tr>
            `).join("")}
        </tbody>
    `;
}

function cellInput(value, handler) {
    const disabled = state.initialized ? "disabled" : "";
    return `<td><input type="number" min="0" value="${Number(value) || 0}" onchange="${handler}" ${disabled}></td>`;
}

function renderProcessCards() {
    const processes = state.serverProcesses.length ? state.serverProcesses : state.processes.map((_, id) => ({
        id,
        status: "WAITING",
        progress: 0
    }));

    document.getElementById("processCards").innerHTML = processes.map(process => {
        const id = process.id;
        const status = (process.status || "WAITING").toLowerCase();
        return `
            <article class="process-card ${status}">
                <div class="process-head">
                    <span class="process-id">P${id}</span>
                    <span class="status-pill">${process.status || "WAITING"}</span>
                </div>
                <div class="process-detail">
                    <span><b>Allocated</b><em>${state.allocation[id]?.join(", ") || ""}</em></span>
                    <span><b>Need</b><em>${state.resources.map((_, j) => getNeed(id, j)).join(", ")}</em></span>
                </div>
                <div class="bar"><div class="bar-fill" style="width:${clamp(process.progress || 0, 0, 100)}%"></div></div>
            </article>
        `;
    }).join("");
}

function renderSafeSequence() {
    const container = document.getElementById("safeSequence");
    if (!state.safeSequence.length) {
        container.innerHTML = `<p class="empty-state">No safe sequence yet.</p>`;
        return;
    }

    container.innerHTML = state.safeSequence.map((processId, index) => {
        const className = index < state.currentStep ? "done" : index === state.currentStep ? "active" : "";
        return `<div class="sequence-step ${className}">P${processId}</div>`;
    }).join("");
}

function renderTimeline() {
    const container = document.getElementById("timelinePanel");
    if (!state.timeline.length) {
        container.innerHTML = `<p class="empty-state">No events yet.</p>`;
        return;
    }

    container.innerHTML = [...state.timeline].reverse().map(event => `
        <div class="timeline-item ${event.safe ? "safe" : "unsafe"}">
            <div class="timeline-dot"></div>
            <div>
                <div class="timeline-head">
                    <strong>${event.event}</strong>
                    <span class="muted">${formatTime(event.timestamp)}</span>
                </div>
                <div class="muted">
                    ${event.processID >= 0 ? `Process P${event.processID} | ` : ""}
                    Available [${(event.available || []).join(", ")}]
                </div>
            </div>
        </div>
    `).join("");
}

function clearTimeline() {
    state.timeline = [];
    renderTimeline();
    showToast("Timeline cleared locally.", "info");
}

function updateAllocation(processIndex, resourceIndex, value) {
    if (guardEdit()) return;
    state.allocation[processIndex][resourceIndex] = sanitizeNumber(value);
    state.need = [];
    keepMaxAtLeastAllocation(processIndex, resourceIndex);
    renderAll();
}

function updateMax(processIndex, resourceIndex, value) {
    if (guardEdit()) return;
    state.max[processIndex][resourceIndex] = sanitizeNumber(value);
    state.need = [];
    renderAll();
}

function updateAvailable(resourceIndex, value) {
    if (guardEdit()) return;
    state.available[resourceIndex] = sanitizeNumber(value);
    renderAll();
}

function updateTotal(resourceIndex, value) {
    if (guardEdit()) return;
    state.totalResources[resourceIndex] = sanitizeNumber(value);
    renderAll();
}

function guardEdit() {
    if (!state.initialized) return false;
    showToast("Use New Config before editing matrices.", "warning");
    renderAll();
    return true;
}

function setMatrixView(view, button) {
    matrixView = view;
    document.querySelectorAll(".matrix-tabs .tab").forEach(tab => tab.classList.remove("active"));
    button.classList.add("active");
    applyMatrixView();
}

function applyMatrixView() {
    const blocks = Array.from(document.querySelectorAll(".matrix-block"));
    const viewMap = {
        all: ["Resource Pool", "Allocation Matrix", "Maximum Matrix", "Need Matrix"],
        resources: ["Resource Pool"],
        allocation: ["Allocation Matrix"],
        maximum: ["Maximum Matrix"],
        need: ["Need Matrix"]
    };
    const visible = viewMap[matrixView] || viewMap.all;

    blocks.forEach(block => {
        const title = block.querySelector("h3")?.textContent || "";
        block.classList.toggle("is-hidden", !visible.includes(title));
    });

    document.getElementById("matrixLayout").style.gridTemplateColumns = matrixView === "all" ? "" : "1fr";
}

function addProcess() {
    if (guardEdit()) return;
    const index = state.processes.length;
    state.processes.push(`P${index}`);
    state.allocation.push(new Array(state.resources.length).fill(0));
    state.max.push(new Array(state.resources.length).fill(0));
    state.need = [];
    renderAll();
}

function removeLastProcess() {
    if (guardEdit()) return;
    if (state.processes.length <= 1) {
        showToast("At least one process is required.", "error");
        return;
    }
    state.processes.pop();
    state.allocation.pop();
    state.max.pop();
    state.need = [];
    renderAll();
}

function addResource() {
    if (guardEdit()) return;
    if (state.resources.length >= 26) {
        showToast("Maximum resource count reached.", "error");
        return;
    }
    const name = String.fromCharCode(65 + state.resources.length);
    state.resources.push(name);
    state.available.push(0);
    state.totalResources.push(0);
    state.allocation.forEach(row => row.push(0));
    state.max.forEach(row => row.push(0));
    state.need = [];
    renderAll();
}

function removeLastResource() {
    if (guardEdit()) return;
    if (state.resources.length <= 1) {
        showToast("At least one resource is required.", "error");
        return;
    }
    state.resources.pop();
    state.available.pop();
    state.totalResources.pop();
    state.allocation.forEach(row => row.pop());
    state.max.forEach(row => row.pop());
    state.need = [];
    renderAll();
}

function validateConfiguration() {
    const error = getValidationMessages().find(item => item.type === "error");
    return error ? { ok: false, message: error.message } : { ok: true };
}

function getValidationMessages() {
    const messages = [];

    for (let i = 0; i < state.processes.length; i++) {
        for (let j = 0; j < state.resources.length; j++) {
            if (state.allocation[i][j] > state.max[i][j]) {
                messages.push({
                    type: "error",
                    title: `P${i} exceeds maximum`,
                    message: `Allocation for resource ${state.resources[j]} is greater than the maximum claim.`
                });
            }
        }
    }

    for (let j = 0; j < state.resources.length; j++) {
        const allocated = state.allocation.reduce((sum, row) => sum + row[j], 0);
        const total = state.totalResources[j];
        const available = state.available[j];

        if (allocated + available > total) {
            messages.push({
                type: "error",
                title: `Resource ${state.resources[j]} is overcommitted`,
                message: `Allocated (${allocated}) plus available (${available}) exceeds total (${total}).`
            });
        } else if (total === 0) {
            messages.push({
                type: "warning",
                title: `Resource ${state.resources[j]} has zero capacity`,
                message: "This resource type cannot be granted until its total capacity is increased."
            });
        }
    }

    if (!messages.length) {
        messages.push({
            type: "success",
            title: "Configuration is internally consistent",
            message: "The resource pool covers current allocations and every process respects its maximum claim."
        });
    }

    return messages.slice(0, 5);
}

function enableControls(enabled) {
    ["runBtn", "pauseBtn", "resumeBtn", "stepBtn", "faultBtn", "recoverBtn"].forEach(id => {
        document.getElementById(id).disabled = !enabled;
    });
    updateControlState();
}

function updateControlState() {
    const initialized = state.initialized;
    const running = state.simulationState === "RUNNING";
    const paused = state.simulationState === "PAUSED";
    const done = state.simulationState === "COMPLETED";

    document.getElementById("runBtn").disabled = !initialized || running || done;
    document.getElementById("pauseBtn").disabled = !initialized || !running;
    document.getElementById("resumeBtn").disabled = !initialized || !paused;
    document.getElementById("stepBtn").disabled = !initialized || (!running && !paused) || done;
    document.getElementById("faultBtn").disabled = !initialized || !running;
    document.getElementById("recoverBtn").disabled = !initialized;
}

function setupThemeToggle() {
    const savedTheme = localStorage.getItem("theme") || "light";
    document.documentElement.setAttribute("data-theme", savedTheme);
    updateThemeButton(savedTheme);

    document.getElementById("themeToggle").addEventListener("click", () => {
        const current = document.documentElement.getAttribute("data-theme");
        const next = current === "light" ? "dark" : "light";
        document.documentElement.setAttribute("data-theme", next);
        localStorage.setItem("theme", next);
        updateThemeButton(next);
    });
}

function updateThemeButton(theme) {
    document.getElementById("themeToggle").textContent = theme === "light" ? "Dark" : "Light";
}

function showToast(message, type = "info") {
    const container = document.getElementById("toastContainer");
    const toast = document.createElement("div");
    toast.className = `toast ${type}`;
    toast.textContent = message;
    container.appendChild(toast);
    setTimeout(() => toast.remove(), 3600);
}

function showLoading(show) {
    document.getElementById("loadingOverlay").classList.toggle("active", show);
}

function getNeed(i, j) {
    if (state.need?.[i]?.[j] !== undefined) {
        return Math.max(0, state.need[i][j]);
    }
    return Math.max(0, (state.max[i]?.[j] || 0) - (state.allocation[i]?.[j] || 0));
}

function keepMaxAtLeastAllocation(i, j) {
    if (state.max[i][j] < state.allocation[i][j]) {
        state.max[i][j] = state.allocation[i][j];
    }
}

function sanitizeNumber(value) {
    const number = Number(value);
    return Number.isFinite(number) && number > 0 ? Math.floor(number) : 0;
}

function normalizeVector(vector, length, fallback = []) {
    return Array.from({ length }, (_, index) => sanitizeNumber(vector?.[index] ?? fallback?.[index] ?? 0));
}

function normalizeMatrix(matrix, rows, cols) {
    return Array.from({ length: rows }, (_, i) => normalizeVector(matrix?.[i], cols));
}

function clamp(value, min, max) {
    return Math.min(max, Math.max(min, value));
}

function formatState(value) {
    return String(value || "IDLE").replace("_", " ").toLowerCase().replace(/\b\w/g, c => c.toUpperCase());
}

function getStatusMessage() {
    const messages = {
        IDLE: "Edit the configuration, initialize the system, then run or step through the sequence.",
        INITIALIZED: "System is initialized. Run the simulation or step through the safe sequence.",
        RUNNING: "Simulation is running. You can pause, inject faults, or watch the automatic steps.",
        PAUSED: "Simulation is paused. Resume or execute the next step manually.",
        UNSAFE: "Unsafe state detected. No safe sequence exists; this is the deadlock-risk result to explain.",
        COMPLETED: "All runnable processes completed successfully."
    };
    return messages[state.simulationState] || "System state updated.";
}

function getStateColor(value) {
    if (value === "UNSAFE") return "var(--danger)";
    if (value === "RUNNING") return "var(--info)";
    if (value === "COMPLETED" || value === "INITIALIZED") return "var(--success)";
    if (value === "PAUSED") return "var(--warning)";
    return "var(--primary)";
}

function formatTime(timestamp) {
    if (!timestamp) return "";
    return new Date(timestamp).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
}
