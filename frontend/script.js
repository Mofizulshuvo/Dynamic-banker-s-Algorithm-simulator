// API Base URL
const API_BASE = 'http://localhost:8080/api';

// Global State
let state = {
    processes: ['P0', 'P1', 'P2', 'P3', 'P4'],
    resources: ['A', 'B', 'C'],
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
    available: [3, 3, 2],
    totalResources: [10, 5, 7],
    initialized: false,
    simulationSpeed: 1000
};

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    initializeUI();
    checkServerStatus();
});

function initializeUI() {
    renderResourceBars();
    renderProcessCards();
    renderAllocation();
    renderMax();
    renderNeed();
    setupThemeToggle();
}

// Check Server Status
async function checkServerStatus() {
    try {
        const response = await fetch(`${API_BASE}/status`);
        if (response.ok) {
            const data = await response.json();
            if (data.success && data.state.simulationState !== 'IDLE') {
                state.initialized = true;
                updateUIFromState(data.state);
                enableControls(true);
            }
        }
    } catch (error) {
        console.log('Server not available yet');
    }
}

// API Helper Functions
async function apiCall(endpoint, method = 'GET', body = null) {
    try {
        const options = {
            method,
            headers: { 'Content-Type': 'application/json' }
        };
        if (body) {
            options.body = JSON.stringify(body);
        }
        const response = await fetch(`${API_BASE}${endpoint}`, options);
        
        if (!response.ok) {
            const errorText = await response.text();
            console.error('API Error:', response.status, errorText);
            throw new Error(`HTTP ${response.status}: ${errorText}`);
        }
        
        return await response.json();
    } catch (error) {
        console.error('API call failed:', error);
        showToast('API call failed: ' + error.message, 'error');
        return { success: false, error: error.message };
    }
}

// Initialize System
async function initializeSystem() {
    showLoading(true);
    
    const body = {
        processCount: state.processes.length,
        resourceCount: state.resources.length,
        allocation: state.allocation,
        maximum: state.max,
        available: state.available,
        totalResources: state.totalResources
    };
    
    const result = await apiCall('/initialize', 'POST', body);
    
    showLoading(false);
    
    if (result.success) {
        state.initialized = true;
        updateUIFromState(result.state);
        enableControls(true);
        showToast('System initialized successfully', 'success');
    } else {
        showToast('Initialization failed: ' + (result.error || 'Unknown error'), 'error');
    }
}

// Run Simulation
async function runSimulation() {
    if (!state.initialized) {
        showToast('Please initialize the system first', 'error');
        return;
    }
    
    showLoading(true);
    
    const result = await apiCall('/run', 'POST');
    
    showLoading(false);
    
    if (result.success) {
        updateUIFromState(result.state);
        showToast('Simulation started', 'success');
        document.getElementById('safeSequenceCard').style.display = 'block';
        renderSafeSequence(result.state.safeSequence);
    } else {
        showToast('Cannot start simulation: ' + result.message, 'error');
    }
}

// Step Simulation
async function stepSimulation() {
    if (!state.initialized) {
        showToast('Please initialize the system first', 'error');
        return;
    }
    
    showLoading(true);
    
    const result = await apiCall('/step', 'POST');
    
    showLoading(false);
    
    if (result.success) {
        updateUIFromState(result.state);
        renderSafeSequence(result.state.safeSequence);
        renderTimeline(result.state.timeline);
    } else {
        showToast('Step failed: ' + result.message, 'error');
    }
}

// Pause Simulation
async function pauseSimulation() {
    const result = await apiCall('/pause', 'POST');
    if (result.success) {
        updateUIFromState(result.state);
        showToast('Simulation paused', 'info');
    }
}

// Resume Simulation
async function resumeSimulation() {
    const result = await apiCall('/resume', 'POST');
    if (result.success) {
        updateUIFromState(result.state);
        showToast('Simulation resumed', 'info');
    }
}

// Reset System
async function resetSystem() {
    const result = await apiCall('/reset', 'POST');
    if (result.success) {
        state.initialized = false;
        updateUIFromState(result.state);
        enableControls(false);
        document.getElementById('safeSequenceCard').style.display = 'none';
        document.getElementById('timelinePanel').innerHTML = '<p style="color: var(--text-secondary); text-align: center; padding: 20px;">No events yet</p>';
        showToast('System reset', 'success');
    }
}

// Update Speed
async function updateSpeed(speed) {
    document.getElementById('speedValue').textContent = speed + 'ms';
    state.simulationSpeed = parseInt(speed);
    
    const result = await apiCall('/speed', 'POST', { speed: parseInt(speed) });
    if (result.success) {
        showToast('Simulation speed updated', 'info');
    }
}

// Show Fault Panel
function showFaultPanel() {
    const panel = document.getElementById('faultPanel');
    panel.style.display = 'block';
    
    // Populate resource select
    const select = document.getElementById('faultResourceID');
    select.innerHTML = state.resources.map((r, i) => 
        `<option value="${i}">${r}</option>`
    ).join('');
}

// Hide Fault Panel
function hideFaultPanel() {
    document.getElementById('faultPanel').style.display = 'none';
}

// Inject Fault
async function injectFault() {
    const type = document.getElementById('faultType').value;
    const resourceID = parseInt(document.getElementById('faultResourceID').value);
    const unitsLost = parseInt(document.getElementById('faultUnitsLost').value);
    
    showLoading(true);
    
    const result = await apiCall('/fault', 'POST', { type, resourceID, unitsLost });
    
    showLoading(false);
    
    if (result.success) {
        updateUIFromState(result.state);
        renderTimeline(result.state.timeline);
        hideFaultPanel();
        showToast('Fault injected: ' + result.fault.description, 'warning');
    } else {
        showToast('Fault injection failed: ' + result.fault.description, 'error');
    }
}

// Show Recovery Panel
function showRecoveryPanel() {
    const panel = document.getElementById('recoveryPanel');
    panel.style.display = 'block';
    
    // Generate recovery options
    const options = document.getElementById('recoveryOptions');
    options.innerHTML = `
        <div class="recovery-option-card">
            <div class="recovery-option-header">
                <span class="recovery-option-title">Restore Resource</span>
            </div>
            <div class="recovery-option-description">Restore lost resources to the available pool</div>
            <div class="recovery-option-form">
                <div class="form-row">
                    <label>Resource ID:</label>
                    <select id="recoveryResourceID">
                        ${state.resources.map((r, i) => `<option value="${i}">${r}</option>`).join('')}
                    </select>
                </div>
                <div class="form-row">
                    <label>Units:</label>
                    <input type="number" id="recoveryUnits" value="1" min="1">
                </div>
            </div>
            <div class="recovery-option-actions">
                <button class="btn btn-success" onclick="applyRecovery('RESTORE_RESOURCE')">Apply</button>
            </div>
        </div>
        
        <div class="recovery-option-card">
            <div class="recovery-option-header">
                <span class="recovery-option-title">Suspend Process</span>
            </div>
            <div class="recovery-option-description">Temporarily suspend a process to free its resources</div>
            <div class="recovery-option-form">
                <div class="form-row">
                    <label>Process ID:</label>
                    <select id="suspendProcessID">
                        ${state.processes.map((p, i) => `<option value="${i}">${p}</option>`).join('')}
                    </select>
                </div>
            </div>
            <div class="recovery-option-actions">
                <button class="btn btn-warning" onclick="applyRecovery('SUSPEND_PROCESS')">Apply</button>
            </div>
        </div>
        
        <div class="recovery-option-card">
            <div class="recovery-option-header">
                <span class="recovery-option-title">Terminate Process</span>
            </div>
            <div class="recovery-option-description">Permanently terminate a process and release all its resources</div>
            <div class="recovery-option-form">
                <div class="form-row">
                    <label>Process ID:</label>
                    <select id="terminateProcessID">
                        ${state.processes.map((p, i) => `<option value="${i}">${p}</option>`).join('')}
                    </select>
                </div>
            </div>
            <div class="recovery-option-actions">
                <button class="btn btn-danger" onclick="applyRecovery('TERMINATE_PROCESS')">Apply</button>
            </div>
        </div>
    `;
}

// Hide Recovery Panel
function hideRecoveryPanel() {
    document.getElementById('recoveryPanel').style.display = 'none';
}

// Apply Recovery
async function applyRecovery(type) {
    let body = { type };
    
    switch (type) {
        case 'RESTORE_RESOURCE':
            body.resourceID = parseInt(document.getElementById('recoveryResourceID').value);
            body.units = parseInt(document.getElementById('recoveryUnits').value);
            break;
        case 'SUSPEND_PROCESS':
            body.processID = parseInt(document.getElementById('suspendProcessID').value);
            break;
        case 'TERMINATE_PROCESS':
            body.processID = parseInt(document.getElementById('terminateProcessID').value);
            break;
    }
    
    showLoading(true);
    
    const result = await apiCall('/recover', 'POST', body);
    
    showLoading(false);
    
    if (result.success) {
        updateUIFromState(result.state);
        renderTimeline(result.state.timeline);
        hideRecoveryPanel();
        showToast('Recovery applied: ' + result.recovery.message, 'success');
    } else {
        showToast('Recovery failed: ' + result.recovery.message, 'error');
    }
}

// Update UI from State
function updateUIFromState(serverState) {
    state.allocation = serverState.allocation;
    state.max = serverState.maximum;
    state.available = serverState.available;
    state.totalResources = serverState.totalResources || state.available;
    
    renderResourceBars();
    renderProcessCards(serverState.processes);
    renderAllocation();
    renderMax();
    renderNeed();
    updateSystemStatus(serverState.simulationState);
}

// Enable/Disable Controls
function enableControls(enabled) {
    document.getElementById('runBtn').disabled = !enabled;
    document.getElementById('pauseBtn').disabled = !enabled;
    document.getElementById('resumeBtn').disabled = !enabled;
    document.getElementById('stepBtn').disabled = !enabled;
    document.getElementById('faultBtn').disabled = !enabled;
    document.getElementById('recoverBtn').disabled = !enabled;
}

// Render Resource Bars
function renderResourceBars() {
    const container = document.getElementById('resourceBars');
    container.innerHTML = state.resources.map((resource, index) => {
        const available = state.available[index];
        const total = state.totalResources[index] || available + 5;
        const percentage = (available / total) * 100;
        
        return `
            <div class="resource-bar-item">
                <div class="resource-bar-header">
                    <span class="resource-bar-label">Resource ${resource}</span>
                    <span class="resource-bar-value">${available} / ${total}</span>
                </div>
                <div class="resource-bar-track">
                    <div class="resource-bar-fill" style="width: ${percentage}%"></div>
                </div>
            </div>
        `;
    }).join('');
}

// Render Process Cards
function renderProcessCards(processes) {
    const container = document.getElementById('processCards');
    
    if (!processes || processes.length === 0) {
        container.innerHTML = state.processes.map((_, i) => `
            <div class="process-card status-waiting">
                <div class="process-card-header">
                    <span class="process-card-id">P${i}</span>
                    <span class="process-card-status">WAITING</span>
                </div>
                <div class="process-card-details">
                    <div class="process-card-detail">
                        <span class="process-card-detail-label">Allocated:</span>
                        <span class="process-card-detail-value">${state.allocation[i].join(', ')}</span>
                    </div>
                    <div class="process-card-detail">
                        <span class="process-card-detail-label">Need:</span>
                        <span class="process-card-detail-value">${state.max[i].map((m, j) => Math.max(0, m - state.allocation[i][j])).join(', ')}</span>
                    </div>
                </div>
            </div>
        `).join('');
        return;
    }
    
    container.innerHTML = processes.map((process, i) => {
        const statusMap = {
            'WAITING': 'waiting',
            'RUNNING': 'running',
            'FINISHED': 'finished',
            'SUSPENDED': 'suspended',
            'TERMINATED': 'terminated'
        };
        const statusClass = statusMap[process.status] || 'waiting';
        
        return `
            <div class="process-card status-${statusClass}">
                <div class="process-card-header">
                    <span class="process-card-id">P${process.id}</span>
                    <span class="process-card-status">${process.status}</span>
                </div>
                <div class="process-card-details">
                    <div class="process-card-detail">
                        <span class="process-card-detail-label">Allocated:</span>
                        <span class="process-card-detail-value">${state.allocation[i].join(', ')}</span>
                    </div>
                    <div class="process-card-detail">
                        <span class="process-card-detail-label">Need:</span>
                        <span class="process-card-detail-value">${state.max[i].map((m, j) => Math.max(0, m - state.allocation[i][j])).join(', ')}</span>
                    </div>
                </div>
                ${process.progress > 0 ? `
                    <div class="process-card-progress">
                        <div class="progress-bar">
                            <div class="progress-fill" style="width: ${process.progress}%"></div>
                        </div>
                    </div>
                ` : ''}
            </div>
        `;
    }).join('');
}

// Render Safe Sequence
function renderSafeSequence(sequence) {
    const container = document.getElementById('safeSequence');
    if (!sequence || sequence.length === 0) {
        container.innerHTML = '<p style="color: var(--text-secondary);">No safe sequence available</p>';
        return;
    }
    
    container.innerHTML = sequence.map((processId, index) => `
        <div class="sequence-step" style="animation-delay: ${index * 0.1}s">P${processId}</div>
    `).join('');
}

// Render Timeline
function renderTimeline(timeline) {
    const container = document.getElementById('timelinePanel');
    
    if (!timeline || timeline.length === 0) {
        container.innerHTML = '<p style="color: var(--text-secondary); text-align: center; padding: 20px;">No events yet</p>';
        return;
    }
    
    container.innerHTML = timeline.map(event => `
        <div class="timeline-item ${event.safe ? 'safe' : 'unsafe'}">
            <div class="timeline-marker"></div>
            <div class="timeline-content">
                <div class="timeline-event">${event.event}</div>
                <div class="timeline-time">Process: P${event.processID}</div>
                <div class="timeline-available">Available: [${event.available.join(', ')}]</div>
            </div>
        </div>
    `).join('');
}

// Clear Timeline
function clearTimeline() {
    document.getElementById('timelinePanel').innerHTML = '<p style="color: var(--text-secondary); text-align: center; padding: 20px;">No events yet</p>';
    showToast('Timeline cleared', 'info');
}

// Theme Toggle
function setupThemeToggle() {
    const themeToggle = document.getElementById('themeToggle');
    const savedTheme = localStorage.getItem('theme') || 'light';
    document.documentElement.setAttribute('data-theme', savedTheme);
    themeToggle.className = `theme-toggle active-${savedTheme}`;

    themeToggle.addEventListener('click', () => {
        const currentTheme = document.documentElement.getAttribute('data-theme');
        const newTheme = currentTheme === 'light' ? 'dark' : 'light';
        document.documentElement.setAttribute('data-theme', newTheme);
        themeToggle.className = `theme-toggle active-${newTheme}`;
        localStorage.setItem('theme', newTheme);
    });
}

// Update System Status
function updateSystemStatus(simulationState) {
    const statusContainer = document.getElementById('systemStatus');
    const statusMap = {
        'IDLE': 'Not initialized',
        'INITIALIZED': 'Ready',
        'SAFE': 'SAFE',
        'UNSAFE': 'UNSAFE',
        'RUNNING': 'Running',
        'PAUSED': 'Paused',
        'FAULT': 'Fault detected',
        'RECOVERY': 'Recovering',
        'COMPLETED': 'Completed'
    };
    
    const statusClass = simulationState === 'SAFE' ? 'status-safe' : 
                       simulationState === 'UNSAFE' ? 'status-unsafe' : 'status-warning';
    
    statusContainer.innerHTML = `
        <div class="status-badge ${statusClass}" style="margin-bottom: 16px;">
            ${simulationState === 'SAFE' ? '✅' : simulationState === 'UNSAFE' ? '⚠️' : '⏳'} ${statusMap[simulationState] || simulationState}
        </div>
        <p style="color: var(--text-secondary);">
            ${simulationState === 'SAFE' ? 'System is in safe state. All processes can complete without deadlock.' :
              simulationState === 'UNSAFE' ? 'System is in unsafe state. Deadlock may occur. Use recovery options.' :
              simulationState === 'RUNNING' ? 'Simulation is running. Use controls to pause or step.' :
              'Ready to start simulation.'}
        </p>
    `;
}

// Toast Notifications
function showToast(message, type = 'info') {
    const container = document.getElementById('toastContainer');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    
    const icons = {
        success: '✅',
        error: '❌',
        warning: '⚠️',
        info: 'ℹ️'
    };
    
    toast.innerHTML = `
        <span class="toast-icon">${icons[type]}</span>
        <span>${message}</span>
    `;
    
    container.appendChild(toast);
    
    setTimeout(() => {
        toast.style.animation = 'slideIn 0.3s ease reverse';
        setTimeout(() => toast.remove(), 300);
    }, 3000);
}

// Loading Animation
function showLoading(show) {
    const overlay = document.getElementById('loadingOverlay');
    if (show) {
        overlay.classList.add('active');
    } else {
        overlay.classList.remove('active');
    }
}

// Add Process
function addProcess() {
    if (state.initialized) {
        showToast('Cannot add processes after initialization. Reset system first.', 'error');
        return;
    }
    
    const newProcessIndex = state.processes.length;
    state.processes.push(`P${newProcessIndex}`);
    state.allocation.push(new Array(state.resources.length).fill(0));
    state.max.push(new Array(state.resources.length).fill(0));
    
    renderResourceBars();
    renderProcessCards();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`Process P${newProcessIndex} added`, 'success');
}

// Remove Process
function removeLastProcess() {
    if (state.initialized) {
        showToast('Cannot remove processes after initialization. Reset system first.', 'error');
        return;
    }
    
    if (state.processes.length <= 1) {
        showToast('Cannot remove the last process', 'error');
        return;
    }
    
    const removedProcess = state.processes.pop();
    state.allocation.pop();
    state.max.pop();
    
    renderResourceBars();
    renderProcessCards();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`${removedProcess} removed`, 'success');
}

// Add Resource
function addResource() {
    if (state.initialized) {
        showToast('Cannot add resources after initialization. Reset system first.', 'error');
        return;
    }
    
    const resourceNames = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const newResourceIndex = state.resources.length;
    if (newResourceIndex >= resourceNames.length) {
        showToast('Maximum resource types reached', 'error');
        return;
    }
    
    const newResource = resourceNames[newResourceIndex];
    state.resources.push(newResource);
    state.available.push(0);
    state.totalResources.push(5);
    state.allocation.forEach(row => row.push(0));
    state.max.forEach(row => row.push(0));
    
    renderResourceBars();
    renderProcessCards();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`Resource ${newResource} added`, 'success');
}

// Remove Resource
function removeLastResource() {
    if (state.initialized) {
        showToast('Cannot remove resources after initialization. Reset system first.', 'error');
        return;
    }
    
    if (state.resources.length <= 1) {
        showToast('Cannot remove the last resource', 'error');
        return;
    }
    
    const removedResource = state.resources.pop();
    state.available.pop();
    state.totalResources.pop();
    state.allocation.forEach(row => row.pop());
    state.max.forEach(row => row.pop());
    
    renderResourceBars();
    renderProcessCards();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`Resource ${removedResource} removed`, 'success');
}
