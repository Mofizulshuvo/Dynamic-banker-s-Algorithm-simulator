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
    history: []
};

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    initializeUI();
    loadFromLocalStorage();
});

function initializeUI() {
    renderAvailable();
    renderAllocation();
    renderMax();
    renderNeed();
    setupThemeToggle();
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

// Render Available Resources
function renderAvailable() {
    const container = document.getElementById('availableDisplay');
    container.innerHTML = state.resources.map((resource, index) => `
        <div class="available-item">
            <div class="label">${resource}</div>
            <input type="number" 
                   value="${state.available[index]}" 
                   min="0" 
                   onchange="updateAvailable(${index}, this.value)"
                   oninput="updateAvailable(${index}, this.value)">
        </div>
    `).join('');
}

function updateAvailable(index, value) {
    const numValue = parseInt(value) || 0;
    if (numValue < 0) {
        showToast('Available resources cannot be negative', 'error');
        return;
    }
    state.available[index] = numValue;
    renderNeed();
}

// Render Allocation Matrix
function renderAllocation() {
    const table = document.getElementById('allocationTable');
    let html = '<thead><tr><th>Process</th>';
    state.resources.forEach(resource => {
        html += `<th class="resource-header">${resource}</th>`;
    });
    html += '</tr></thead><tbody>';

    state.processes.forEach((process, pIndex) => {
        html += `<tr><td class="process-header">${process}</td>`;
        state.resources.forEach((_, rIndex) => {
            html += `<td><input type="number" 
                           value="${state.allocation[pIndex][rIndex]}" 
                           min="0" 
                           onchange="updateAllocation(${pIndex}, ${rIndex}, this.value)"
                           oninput="updateAllocation(${pIndex}, ${rIndex}, this.value)"></td>`;
        });
        html += '</tr>';
    });

    html += '</tbody>';
    table.innerHTML = html;
}

function updateAllocation(pIndex, rIndex, value) {
    const numValue = parseInt(value) || 0;
    if (numValue < 0) {
        showToast('Allocation cannot be negative', 'error');
        return;
    }
    state.allocation[pIndex][rIndex] = numValue;
    renderNeed();
}

// Render Max Matrix
function renderMax() {
    const table = document.getElementById('maxTable');
    let html = '<thead><tr><th>Process</th>';
    state.resources.forEach(resource => {
        html += `<th class="resource-header">${resource}</th>`;
    });
    html += '</tr></thead><tbody>';

    state.processes.forEach((process, pIndex) => {
        html += `<tr><td class="process-header">${process}</td>`;
        state.resources.forEach((_, rIndex) => {
            html += `<td><input type="number" 
                           value="${state.max[pIndex][rIndex]}" 
                           min="0" 
                           onchange="updateMax(${pIndex}, ${rIndex}, this.value)"
                           oninput="updateMax(${pIndex}, ${rIndex}, this.value)"></td>`;
        });
        html += '</tr>';
    });

    html += '</tbody>';
    table.innerHTML = html;
}

function updateMax(pIndex, rIndex, value) {
    const numValue = parseInt(value) || 0;
    if (numValue < 0) {
        showToast('Max cannot be negative', 'error');
        return;
    }
    state.max[pIndex][rIndex] = numValue;
    renderNeed();
}

// Render Need Matrix (Auto-calculated)
function renderNeed() {
    const table = document.getElementById('needTable');
    let html = '<thead><tr><th>Process</th>';
    state.resources.forEach(resource => {
        html += `<th class="resource-header">${resource}</th>`;
    });
    html += '</tr></thead><tbody>';

    state.processes.forEach((process, pIndex) => {
        html += `<tr><td class="process-header">${process}</td>`;
        state.resources.forEach((_, rIndex) => {
            const need = Math.max(0, state.max[pIndex][rIndex] - state.allocation[pIndex][rIndex]);
            html += `<td style="font-weight: 600; color: ${need > 0 ? 'var(--primary)' : 'var(--success)'}">${need}</td>`;
        });
        html += '</tr>';
    });

    html += '</tbody>';
    table.innerHTML = html;
}

// Add Process
function addProcess() {
    const newProcessIndex = state.processes.length;
    state.processes.push(`P${newProcessIndex}`);
    state.allocation.push(new Array(state.resources.length).fill(0));
    state.max.push(new Array(state.resources.length).fill(0));
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`Process P${newProcessIndex} added`, 'success');
}

// Remove Process
function removeLastProcess() {
    if (state.processes.length <= 1) {
        showToast('Cannot remove the last process', 'error');
        return;
    }
    const removedProcess = state.processes.pop();
    state.allocation.pop();
    state.max.pop();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`${removedProcess} removed`, 'success');
}

// Add Resource
function addResource() {
    const resourceNames = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    const newResourceIndex = state.resources.length;
    if (newResourceIndex >= resourceNames.length) {
        showToast('Maximum resource types reached', 'error');
        return;
    }
    const newResource = resourceNames[newResourceIndex];
    state.resources.push(newResource);
    state.available.push(0);
    state.allocation.forEach(row => row.push(0));
    state.max.forEach(row => row.push(0));
    renderAvailable();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`Resource ${newResource} added`, 'success');
}

// Remove Resource
function removeLastResource() {
    if (state.resources.length <= 1) {
        showToast('Cannot remove the last resource', 'error');
        return;
    }
    const removedResource = state.resources.pop();
    state.available.pop();
    state.allocation.forEach(row => row.pop());
    state.max.forEach(row => row.pop());
    renderAvailable();
    renderAllocation();
    renderMax();
    renderNeed();
    showToast(`Resource ${removedResource} removed`, 'success');
}

// Download Input JSON for C++ Backend
function downloadInput() {
    const n = state.processes.length;
    const m = state.resources.length;
    
    // Format: processes resources
    // Then allocation matrix (n x m)
    // Then max matrix (n x m)
    // Then available vector (m)
    let content = `${n} ${m}\n`;
    
    // Allocation matrix
    state.allocation.forEach(row => {
        content += row.join(' ') + '\n';
    });
    
    // Max matrix
    state.max.forEach(row => {
        content += row.join(' ') + '\n';
    });
    
    // Available vector
    content += state.available.join(' ') + '\n';
    
    const blob = new Blob([content], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'input.json';
    link.click();
    URL.revokeObjectURL(url);
    
    showToast('Input file downloaded. Save it to data/ folder and run C++ backend.', 'info');
}

// Upload Input JSON
function uploadInputFile(event) {
    const file = event.target.files[0];
    if (!file) return;
    
    const reader = new FileReader();
    reader.onload = (e) => {
        try {
            const lines = e.target.result.trim().split('\n');
            const firstLine = lines[0].split(' ');
            const p = parseInt(firstLine[0]);
            const r = parseInt(firstLine[1]);
            
            // Parse allocation matrix
            const allocation = [];
            for (let i = 0; i < p; i++) {
                allocation.push(lines[i + 1].split(' ').map(Number));
            }
            
            // Parse max matrix
            const max = [];
            for (let i = 0; i < p; i++) {
                max.push(lines[p + 1 + i].split(' ').map(Number));
            }
            
            // Parse available vector
            const available = lines[2 * p + 1].split(' ').map(Number);
            
            // Update state
            state.processes = Array.from({length: p}, (_, i) => `P${i}`);
            state.resources = Array.from({length: r}, (_, i) => 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'[i]);
            state.allocation = allocation;
            state.max = max;
            state.available = available;
            
            initializeUI();
            showToast('Input file loaded successfully', 'success');
        } catch (error) {
            showToast('Invalid input file format', 'error');
        }
    };
    reader.readAsText(file);
    event.target.value = '';
}

// Upload Output JSON from C++ Backend
function uploadOutputFile(event) {
    const file = event.target.files[0];
    if (!file) return;
    
    const reader = new FileReader();
    reader.onload = (e) => {
        try {
            const result = JSON.parse(e.target.result);
            displayResults(result);
            addToHistory(result);
            showToast('Output file loaded successfully', 'success');
        } catch (error) {
            showToast('Invalid output file format', 'error');
        }
    };
    reader.readAsText(file);
    event.target.value = '';
}

// Banker's Algorithm Implementation (JavaScript fallback)
function bankersAlgorithmJS() {
    const n = state.processes.length;
    const m = state.resources.length;
    
    // Calculate Need matrix
    const need = state.max.map((maxRow, i) => 
        maxRow.map((maxVal, j) => Math.max(0, maxVal - state.allocation[i][j]))
    );
    
    // Work = Available
    const work = [...state.available];
    const finish = new Array(n).fill(false);
    const safeSequence = [];
    
    // Safety algorithm
    let found;
    do {
        found = false;
        for (let i = 0; i < n; i++) {
            if (!finish[i]) {
                let canAllocate = true;
                for (let j = 0; j < m; j++) {
                    if (need[i][j] > work[j]) {
                        canAllocate = false;
                        break;
                    }
                }
                
                if (canAllocate) {
                    // Process can finish
                    for (let j = 0; j < m; j++) {
                        work[j] += state.allocation[i][j];
                    }
                    finish[i] = true;
                    safeSequence.push(i);
                    found = true;
                }
            }
        }
    } while (found);
    
    const isSafe = safeSequence.length === n;
    
    return {
        isSafe,
        safeSequence: safeSequence.map(i => state.processes[i]),
        need,
        work
    };
}

// Run Simulation (JavaScript fallback)
async function runSimulation() {
    showLoading(true);
    
    // Simulate processing delay for animation
    await new Promise(resolve => setTimeout(resolve, 500));
    
    const result = bankersAlgorithmJS();
    
    showLoading(false);
    displayResults(result);
    addToHistory(result);
    showToast('Simulation completed using JavaScript algorithm', 'info');
}

// Display Results
async function displayResults(result) {
    const resultsContainer = document.getElementById('simulationResults');
    const resultHeader = document.getElementById('resultHeader');
    const resultStatus = document.getElementById('resultStatus');
    const safeSequence = document.getElementById('safeSequence');
    const progressFill = document.getElementById('progressFill');
    
    resultsContainer.classList.add('active');
    
    if (result.isSafe) {
        resultHeader.className = 'result-header safe';
        resultStatus.innerHTML = '✅ SAFE STATE';
        updateSystemStatus('safe');
    } else {
        resultHeader.className = 'result-header unsafe';
        resultStatus.innerHTML = '⚠️ UNSAFE STATE - Deadlock Risk';
        updateSystemStatus('unsafe');
    }
    
    // Animate safe sequence
    if (result.isSafe && result.safeSequence.length > 0) {
        safeSequence.innerHTML = result.safeSequence.map((process, index) => 
            `<div class="sequence-step" id="step-${index}">${process}</div>`
        ).join('');
        
        // Animate steps
        progressFill.style.width = '0%';
        for (let i = 0; i < result.safeSequence.length; i++) {
            await new Promise(resolve => setTimeout(resolve, 300));
            const step = document.getElementById(`step-${i}`);
            step.classList.add('active');
            progressFill.style.width = `${((i + 1) / result.safeSequence.length) * 100}%`;
            
            await new Promise(resolve => setTimeout(resolve, 300));
            step.classList.remove('active');
            step.classList.add('completed');
        }
    } else {
        safeSequence.innerHTML = '<p style="color: var(--text-secondary);">No safe sequence found</p>';
        progressFill.style.width = '0%';
    }
}

// Update System Status
function updateSystemStatus(status) {
    const statusContainer = document.getElementById('systemStatus');
    if (status === 'safe') {
        statusContainer.innerHTML = `
            <div class="status-badge status-safe" style="margin-bottom: 16px;">
                ✅ System is in SAFE state
            </div>
            <p style="color: var(--text-secondary);">All processes can complete without deadlock.</p>
        `;
    } else if (status === 'unsafe') {
        statusContainer.innerHTML = `
            <div class="status-badge status-unsafe" style="margin-bottom: 16px;">
                ⚠️ System is in UNSAFE state
            </div>
            <p style="color: var(--text-secondary);">Deadlock may occur. Review resource allocation.</p>
        `;
    } else {
        statusContainer.innerHTML = `
            <div class="status-badge status-warning" style="margin-bottom: 16px;">
                ⏳ Ready for simulation
            </div>
            <div class="help-section">
                <h3>💡 Quick Guide</h3>
                <p>1. Edit the matrices below or use the controls above</p>
                <p>2. Click "Download Input" to save input.json</p>
                <p>3. Run C++ backend (run.bat or manually)</p>
                <p>4. Click "Upload Output" to load results</p>
                <p>5. Green = Safe, Red = Unsafe (deadlock risk)</p>
            </div>
        `;
    }
}

// Fault Injection
function injectRandomFault() {
    if (state.resources.length === 0) {
        showToast('No resources to inject fault', 'error');
        return;
    }
    
    const randomResource = Math.floor(Math.random() * state.resources.length);
    const faultAmount = Math.floor(Math.random() * 3) + 1;
    
    state.available[randomResource] = Math.max(0, state.available[randomResource] - faultAmount);
    renderAvailable();
    showToast(`Injected fault: -${faultAmount} ${state.resources[randomResource]}`, 'warning');
}

function updateFaultValue(value) {
    document.getElementById('faultValue').textContent = value;
}

function injectManualFault() {
    const faultAmount = parseInt(document.getElementById('faultSlider').value);
    if (faultAmount <= 0) {
        showToast('Set fault amount greater than 0', 'error');
        return;
    }
    
    if (state.resources.length === 0) {
        showToast('No resources to inject fault', 'error');
        return;
    }
    
    // Apply fault to first resource
    state.available[0] = Math.max(0, state.available[0] - faultAmount);
    renderAvailable();
    showToast(`Manual fault applied: -${faultAmount} ${state.resources[0]}`, 'warning');
    
    // Reset slider
    document.getElementById('faultSlider').value = 0;
    document.getElementById('faultValue').textContent = '0';
}

// Reset System
function resetSystem() {
    state = {
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
        history: []
    };
    
    initializeUI();
    document.getElementById('simulationResults').classList.remove('active');
    updateSystemStatus('ready');
    renderHistory();
    showToast('System reset to default state', 'success');
}

// History Management
function addToHistory(result) {
    const timestamp = new Date().toLocaleTimeString();
    state.history.unshift({
        timestamp,
        isSafe: result.isSafe,
        sequence: result.safeSequence
    });
    
    // Keep only last 10 entries
    if (state.history.length > 10) {
        state.history.pop();
    }
    
    renderHistory();
}

function renderHistory() {
    const panel = document.getElementById('historyPanel');
    
    if (state.history.length === 0) {
        panel.innerHTML = '<p style="color: var(--text-secondary); text-align: center; padding: 20px;">No simulations run yet</p>';
        return;
    }
    
    panel.innerHTML = state.history.map(entry => `
        <div class="history-item">
            <div>
                <div class="history-result ${entry.isSafe ? 'safe' : 'unsafe'}">
                    ${entry.isSafe ? '✅ Safe' : '⚠️ Unsafe'}
                </div>
                <div class="history-time">${entry.timestamp}</div>
            </div>
            <div style="font-size: 0.85rem; color: var(--text-secondary);">
                ${entry.sequence.length > 0 ? entry.sequence.join(' → ') : 'No sequence'}
            </div>
        </div>
    `).join('');
}

function clearHistory() {
    state.history = [];
    renderHistory();
    showToast('History cleared', 'success');
}

// Save/Load State
function saveState() {
    localStorage.setItem('bankerState', JSON.stringify(state));
    showToast('State saved to browser', 'success');
}

function loadState() {
    const saved = localStorage.getItem('bankerState');
    if (saved) {
        state = JSON.parse(saved);
        initializeUI();
        renderHistory();
        showToast('State loaded from browser', 'success');
    } else {
        showToast('No saved state found', 'error');
    }
}

function loadFromLocalStorage() {
    const saved = localStorage.getItem('bankerState');
    if (saved) {
        state = JSON.parse(saved);
        initializeUI();
        renderHistory();
    }
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
