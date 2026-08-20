let rows = 10;
let cols = 10;
let start = [0, 0];
let targets = [];
let obstacles = [];
let currentGraph = null;
let currentPaths = [];

const MAX_TARGETS = 8;

function sameCell(left, right)
{
    return left[0] === right[0] && left[1] === right[1];
}

function cellId(cell)
{
    return `${cell[0]}-${cell[1]}`;
}

function isObstacle(cell)
{
    return obstacles.some(obstacle => sameCell(obstacle, cell));
}

function allCells()
{
    const cells = [];
    for (let x = 0; x < rows; x++)
    {
        for (let y = 0; y < cols; y++)
        {
            cells.push([x, y]);
        }
    }
    return cells;
}

function previewGraph()
{
    const graph = { nodes: allCells(), edges: [] };
    for (let x = 0; x < rows; x++)
    {
        for (let y = 0; y < cols; y++)
        {
            const cell = [x, y];
            if (isObstacle(cell))
                continue;

            for (const [nextX, nextY] of [[x + 1, y], [x, y + 1]])
            {
                const next = [nextX, nextY];
                if (nextX < rows && nextY < cols && !isObstacle(next))
                    graph.edges.push({ from: cell, to: next });
            }
        }
    }
    return graph;
}

function createGraph()
{
    rows = Number(document.getElementById("rows").value);
    cols = Number(document.getElementById("cols").value);

    if (!Number.isInteger(rows) || !Number.isInteger(cols) || rows < 2 || cols < 2 || rows > 20 || cols > 20)
    {
        setStatus("Use whole-number graph dimensions between 2 and 20.", "error");
        return;
    }

    start = [0, 0];
    targets = [[rows - 1, cols - 1]];
    obstacles = [];
    currentPaths = [];
    currentGraph = previewGraph();
    drawGraph(currentGraph, currentPaths);
    setStatus("Graph ready. Use Node action to set the start, targets, or blocked nodes.", "info");
}

function updateTargetList()
{
    const list = document.getElementById("target-list");
    const formattedTargets = targets.map((target, index) =>
        `T${index + 1} (${target[0]}, ${target[1]})`).join(" → ");
    list.innerHTML = `<strong>Start:</strong> (${start[0]}, ${start[1]}) &nbsp; <strong>Targets:</strong> ${formattedTargets || "None"}`;
}

function selectNode(cell)
{
    const action = document.getElementById("node-action").value;

    if (action === "start")
    {
        if (isObstacle(cell))
        {
            setStatus("Unblock this node before making it the start.", "error");
            return;
        }
        if (targets.some(target => sameCell(target, cell)))
        {
            setStatus("Remove this target before making it the start.", "error");
            return;
        }
        start = cell;
        setStatus(`Start set to (${cell[0]}, ${cell[1]}).`, "info");
    }
    else if (action === "target")
    {
        if (isObstacle(cell))
        {
            setStatus("Unblock this node before adding it as a target.", "error");
            return;
        }
        if (sameCell(start, cell))
        {
            setStatus("The start node cannot also be a target.", "error");
            return;
        }
        if (targets.some(target => sameCell(target, cell)))
        {
            setStatus("That node is already a target.", "info");
            return;
        }
        if (targets.length === MAX_TARGETS)
        {
            setStatus(`A route can include up to ${MAX_TARGETS} targets.`, "error");
            return;
        }
        targets.push(cell);
        setStatus(`Added target ${targets.length} at (${cell[0]}, ${cell[1]}).`, "info");
    }
    else
    {
        if (sameCell(start, cell) || targets.some(target => sameCell(target, cell)))
        {
            setStatus("Move or remove the start/target before blocking this node.", "error");
            return;
        }
        const obstacleIndex = obstacles.findIndex(obstacle => sameCell(obstacle, cell));
        if (obstacleIndex === -1)
        {
            obstacles.push(cell);
            setStatus(`Blocked node (${cell[0]}, ${cell[1]}).`, "info");
        }
        else
        {
            obstacles.splice(obstacleIndex, 1);
            setStatus(`Unblocked node (${cell[0]}, ${cell[1]}).`, "info");
        }
    }

    currentPaths = [];
    currentGraph = previewGraph();
    drawGraph(currentGraph, currentPaths);
}

function removeLastTarget()
{
    if (targets.length === 0)
    {
        setStatus("There are no targets to remove.", "info");
        return;
    }
    const removed = targets.pop();
    currentPaths = [];
    currentGraph = previewGraph();
    drawGraph(currentGraph, currentPaths);
    setStatus(`Removed target (${removed[0]}, ${removed[1]}).`, "info");
}

function drawGraph(graph, paths)
{
    const container = document.getElementById("graph");
    const edges = graph && Array.isArray(graph.edges) ? graph.edges : [];
    const spacing = 72;
    const padding = 42;
    const width = Math.max(200, cols * spacing + padding * 2);
    const height = Math.max(200, rows * spacing + padding * 2);
    const routeEdges = new Set();

    (paths || []).forEach(path =>
    {
        for (let index = 1; index < path.length; index++)
            routeEdges.add([cellId(path[index - 1]), cellId(path[index])].sort().join("|"));
    });

    const point = cell => ({ x: padding + cell[1] * spacing, y: padding + cell[0] * spacing });
    let svg = `<svg class="graph-svg" viewBox="0 0 ${width} ${height}" width="${width}" height="${height}" role="img" aria-label="Interactive weighted route graph">`;

    edges.forEach(edge =>
    {
        const from = point(edge.from);
        const to = point(edge.to);
        const key = [cellId(edge.from), cellId(edge.to)].sort().join("|");
        svg += `<line class="graph-edge${routeEdges.has(key) ? " route-edge" : ""}" x1="${from.x}" y1="${from.y}" x2="${to.x}" y2="${to.y}"/>`;
        if (Number.isFinite(edge.weight))
        {
            const midX = (from.x + to.x) / 2;
            const midY = (from.y + to.y) / 2;
            svg += `<text class="graph-weight" x="${midX + 4}" y="${midY - 4}">${edge.weight}</text>`;
        }
    });

    allCells().forEach(cell =>
    {
        const position = point(cell);
        const targetIndex = targets.findIndex(target => sameCell(target, cell));
        let nodeClass = "graph-node";
        let labelClass = "graph-node-label";
        let label = `${cell[0]},${cell[1]}`;

        if (sameCell(start, cell))
        {
            nodeClass += " graph-start";
            labelClass += " inverse";
            label = "S";
        }
        else if (targetIndex >= 0)
        {
            nodeClass += ` graph-target ${targetIndex < 4 ? `graph-target-${targetIndex + 1}` : "graph-target-more"}`;
            labelClass += " inverse";
            label = `T${targetIndex + 1}`;
        }
        else if (isObstacle(cell))
        {
            nodeClass += " graph-obstacle";
            labelClass += " inverse";
            label = "×";
        }

        svg += `<g class="graph-node-group" data-x="${cell[0]}" data-y="${cell[1]}">`;
        svg += `<circle class="${nodeClass}" cx="${position.x}" cy="${position.y}" r="17"/>`;
        svg += `<text class="${labelClass}" text-anchor="middle" x="${position.x}" y="${position.y + 4}">${label}</text>`;
        svg += "</g>";
    });

    container.innerHTML = `${svg}</svg>`;
    updateTargetList();
}

function clearRoutes()
{
    currentPaths = [];
    drawGraph(currentGraph || previewGraph(), currentPaths);
}

async function findPath()
{
    clearRoutes();
    if (targets.length === 0)
    {
        setStatus("Add at least one target before finding routes.", "error");
        return;
    }

    const findButton = document.getElementById("find-path");
    findButton.disabled = true;
    findButton.textContent = "Finding routes...";
    setStatus("Calculating routes and random edge weights...", "info");

    const data = {
        rows,
        cols,
        start,
        targets,
        obstacles,
        algorithm: document.getElementById("algorithm").value
    };

    try
    {
        const response = await fetch("/route", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(data)
        });
        const result = await response.json();
        if (!response.ok)
            throw new Error(result.message || "The server rejected this request.");

        currentGraph = result.graph || previewGraph();
        currentPaths = result.targetPaths || (result.path ? [result.path] : []);
        drawGraph(currentGraph, currentPaths);
        showAlgorithmLog(result.algorithmLog);

        if (result.success)
        {
            const targetWord = targets.length === 1 ? "target" : "targets";
            setStatus(`${result.algorithm} route reaches ${targets.length} ${targetWord} in ${result.steps} total steps.`, "success");
        }
        else
        {
            setStatus(result.message || "No route was found.", "error");
        }
    }
    catch (error)
    {
        console.error(error);
        setStatus(error.message || "Backend connection failed. Start the C++ server and try again.", "error");
    }
    finally
    {
        findButton.disabled = false;
        findButton.textContent = "Find Routes";
    }
}

function showAlgorithmLog(log)
{
    const panel = document.getElementById("algorithm-log");
    if (!log || log.length === 0)
    {
        panel.innerHTML = "<h2>Algorithm Explanation Log</h2><p>No explanation is available for this request.</p>";
        return;
    }

    const entries = log.map(entry => `<li>${entry}</li>`).join("");
    panel.innerHTML = `<h2>Algorithm Explanation Log</h2><ol>${entries}</ol>`;
}

function setStatus(message, type)
{
    const status = document.getElementById("status");
    status.textContent = message;
    status.className = type;
}

document.getElementById("create-graph").addEventListener("click", createGraph);
document.getElementById("remove-target").addEventListener("click", removeLastTarget);
document.getElementById("find-path").addEventListener("click", findPath);
document.getElementById("graph").addEventListener("click", event =>
{
    const node = event.target.closest(".graph-node-group");
    if (!node)
        return;
    selectNode([Number(node.dataset.x), Number(node.dataset.y)]);
});

createGraph();
