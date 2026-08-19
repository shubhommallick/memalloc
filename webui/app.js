document.addEventListener('DOMContentLoaded', () => {
    const statTotal = document.getElementById('stat-total');
    const statActive = document.getElementById('stat-active');
    const statPeak = document.getElementById('stat-peak');
    const statCount = document.getElementById('stat-count');

    const barTotal = document.getElementById('bar-total');
    const barActive = document.getElementById('bar-active');
    const barPeak = document.getElementById('bar-peak');

    const badgeArena = document.getElementById('badge-arena');
    const progressArena = document.getElementById('progress-arena');

    const badgePool = document.getElementById('badge-pool');
    const gridPool = document.getElementById('grid-pool');

    const badgeFreeList = document.getElementById('badge-freelist');
    const progressFreeList = document.getElementById('progress-freelist');

    const tableSnapshots = document.getElementById('table-snapshots');

    const btnAllocArena = document.getElementById('btn-alloc-arena');
    const btnAllocPool = document.getElementById('btn-alloc-pool');
    const btnAllocFreeList = document.getElementById('btn-alloc-freelist');
    const btnReset = document.getElementById('btn-reset');

    function formatBytes(bytes) {
        if (bytes === 0) return '0 B';
        const k = 1024;
        const sizes = ['B', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
    }

    async function fetchStats() {
        try {
            const res = await fetch('/api/stats');
            if (!res.ok) return;
            const data = await res.json();

            // Update Header Metrics
            statTotal.textContent = formatBytes(data.totalAllocated);
            statActive.textContent = formatBytes(data.currentActive);
            statPeak.textContent = formatBytes(data.peakMemory);
            statCount.textContent = data.allocationCount;

            const maxCapacity = Math.max(data.peakMemory * 1.5, 8192);
            barTotal.style.width = Math.min((data.totalAllocated / maxCapacity) * 100, 100) + '%';
            barActive.style.width = Math.min((data.currentActive / (data.peakMemory || 1)) * 100, 100) + '%';
            barPeak.style.width = Math.min((data.peakMemory / maxCapacity) * 100, 100) + '%';

            // Arena Allocator
            badgeArena.textContent = `${data.arena.used} / ${data.arena.capacity} B`;
            const arenaPct = (data.arena.used / data.arena.capacity) * 100;
            progressArena.style.width = arenaPct + '%';

            // Pool Allocator Block Grid
            badgePool.textContent = `${data.pool.usedBlocks} / ${data.pool.totalBlocks} Blocks`;
            gridPool.innerHTML = '';
            for (let i = 0; i < data.pool.totalBlocks; i++) {
                const block = document.createElement('div');
                block.className = 'block-item' + (i < data.pool.usedBlocks ? ' used' : '');
                gridPool.appendChild(block);
            }

            // Free List Allocator
            badgeFreeList.textContent = `${data.freeList.used} / ${data.freeList.capacity} B`;
            const freeListPct = (data.freeList.used / data.freeList.capacity) * 100;
            progressFreeList.style.width = freeListPct + '%';

            // Allocation Snapshots Table
            if (!data.snapshots || data.snapshots.length === 0) {
                tableSnapshots.innerHTML = '<tr><td colspan="4" class="text-muted">No active allocations</td></tr>';
            } else {
                tableSnapshots.innerHTML = data.snapshots.map(s => `
                    <tr>
                        <td>${s.address}</td>
                        <td>${s.size} B</td>
                        <td>${s.allocatorType}</td>
                        <td><span class="status-tag ${s.active ? 'active' : 'freed'}">${s.active ? 'ACTIVE' : 'FREED'}</span></td>
                    </tr>
                `).join('');
            }
        } catch (e) {
            console.error("Error fetching allocator stats:", e);
        }
    }

    // Trigger API calls for interactive testing
    btnAllocArena?.addEventListener('click', () => fetch('/api/allocate?allocator=Arena'));
    btnAllocPool?.addEventListener('click', () => fetch('/api/allocate?allocator=Pool'));
    btnAllocFreeList?.addEventListener('click', () => fetch('/api/allocate?allocator=FreeList'));
    btnReset?.addEventListener('click', () => fetch('/api/reset'));

    // Poll every 400ms
    setInterval(fetchStats, 400);
    fetchStats();
});
