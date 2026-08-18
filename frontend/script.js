let rows = 10;
let cols = 10;

let start = [0,0];
let end;

let obstacles = [];


// =========================
// Create Warehouse Grid
// =========================
function createGrid()
{
    rows = Number(document.getElementById("rows").value);
    cols = Number(document.getElementById("cols").value);

    if (!Number.isInteger(rows) || !Number.isInteger(cols) || rows < 2 || cols < 2 || rows > 20 || cols > 20)
    {
        setStatus("Use whole-number grid dimensions between 2 and 20.", "error");
        return;
    }

    end = [rows - 1, cols - 1];

    obstacles = [];
    clearPath();
    document.getElementById("graph").innerHTML = "";
    setStatus("Grid ready. Click cells to add obstacles.", "info");


    let grid = document.getElementById("grid");

    grid.innerHTML = "";


    grid.style.gridTemplateColumns =
        `repeat(${cols},40px)`;


    for(let i = 0; i < rows; i++)
    {
        for(let j = 0; j < cols; j++)
        {
            let cell = document.createElement("div");

            cell.className = "cell";

            cell.id = `${i}-${j}`;


            cell.onclick = function()
            {
                toggleObstacle(i,j);
            };


            grid.appendChild(cell);
        }
    }


    updateCells();
}



// =========================
// Add / Remove Obstacles
// =========================
function toggleObstacle(x,y)
{

    if(
        (x === start[0] && y === start[1]) ||
        (x === end[0] && y === end[1])
    )
    {
        return;
    }


    let index = obstacles.findIndex(
        o => o[0] === x && o[1] === y
    );


    if(index >= 0)
    {
        obstacles.splice(index,1);
    }
    else
    {
        obstacles.push([x,y]);
    }


    updateCells();
}



// =========================
// Update Grid Colors
// =========================
function updateCells()
{

    document.querySelectorAll(".cell")
    .forEach(cell =>
    {
        cell.className = "cell";
    });


    document.getElementById(
        `${start[0]}-${start[1]}`
    ).classList.add("start");


    document.getElementById(
        `${end[0]}-${end[1]}`
    ).classList.add("end");


    obstacles.forEach(o =>
    {
        let cell = document.getElementById(
            `${o[0]}-${o[1]}`
        );

        if(cell)
        {
            cell.classList.add("obstacle");
        }
    });

}



// =========================
// Send Request to Drogon
// =========================
async function findPath()
{
    clearPath();
    if (!Number.isInteger(rows) || !Number.isInteger(cols))
    {
        setStatus("Create a valid grid before finding a path.", "error");
        return;
    }

    const findButton = document.getElementById("find-path");
    findButton.disabled = true;
    findButton.textContent = "Finding route...";
    setStatus("Calculating route and random edge weights...", "info");


    let algorithm =
        document.getElementById("algorithm").value;


    let data =
    {
        rows: rows,
        cols: cols,
        start: start,
        end: end,
        obstacles: obstacles,
        algorithm: algorithm
    };


    try
    {

        let response = await fetch(
            "/route",
            {
                method:"POST",

                headers:
                {
                    "Content-Type":"application/json"
                },

                body: JSON.stringify(data)
            }
        );


        const result = await response.json();
        if (!response.ok)
            throw new Error(result.message || "The server rejected this request.");

        // The graph is useful even when no route can be found.
        drawGraph(result.graph, result.path || []);



        if(result.success)
        {

            drawPath(result.path);
            setStatus(`${result.algorithm} path found in ${result.steps} steps.`, "success");
            showAlgorithmLog(result.algorithmLog);
        }
        else
        {

            setStatus(result.message || "No route was found.", "error");
            showAlgorithmLog(result.algorithmLog);

        }


    }
    catch(error)
    {

        console.error(error);
        setStatus(error.message || "Backend connection failed. Start the C++ server and try again.", "error");
    }
    finally
    {
        findButton.disabled = false;
        findButton.textContent = "Find Path";

    }

}

// Draws the warehouse as a weighted graph: each free cell is a vertex and
// each line is a traversable connection. Edge labels are generated randomly
// by the backend for the current request.
function drawGraph(graph, path)
{
    const container = document.getElementById("graph");
    if (!graph || !graph.nodes)
    {
        container.innerHTML = "<p>No graph data is available.</p>";
        return;
    }

    const spacing = 72;
    const padding = 38;
    const width = Math.max(160, cols * spacing + padding * 2);
    const height = Math.max(160, rows * spacing + padding * 2);
    const routeEdges = new Set();
    for (let index = 1; path && index < path.length; index++)
    {
        const a = `${path[index - 1][0]}-${path[index - 1][1]}`;
        const b = `${path[index][0]}-${path[index][1]}`;
        routeEdges.add([a, b].sort().join("|"));
    }
    const point = cell => ({ x: padding + cell[1] * spacing, y: padding + cell[0] * spacing });
    const nodeId = cell => `${cell[0]}-${cell[1]}`;
    let svg = `<svg class="graph-svg" viewBox="0 0 ${width} ${height}" width="${width}" height="${height}" role="img" aria-label="Weighted warehouse graph">`;

    graph.edges.forEach(edge => {
        const from = point(edge.from);
        const to = point(edge.to);
        const key = [nodeId(edge.from), nodeId(edge.to)].sort().join("|");
        const midX = (from.x + to.x) / 2;
        const midY = (from.y + to.y) / 2;
        svg += `<line class="graph-edge${routeEdges.has(key) ? " route-edge" : ""}" x1="${from.x}" y1="${from.y}" x2="${to.x}" y2="${to.y}"/>`;
        svg += `<text class="graph-weight" x="${midX + 4}" y="${midY - 4}">${edge.weight}</text>`;
    });
    graph.nodes.forEach(cell => {
        const position = point(cell);
        let extra = "";
        if (cell[0] === start[0] && cell[1] === start[1]) extra = " graph-start";
        if (cell[0] === end[0] && cell[1] === end[1]) extra = " graph-end";
        svg += `<circle class="graph-node${extra}" cx="${position.x}" cy="${position.y}" r="15"/>`;
        svg += `<text class="graph-node-label" text-anchor="middle" x="${position.x}" y="${position.y + 4}">${cell[0]},${cell[1]}</text>`;
    });
    container.innerHTML = `${svg}</svg>`;
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



// =========================
// Draw Path
// =========================
function drawPath(path)
{

    path.forEach(point =>
    {

        let id =
        `${point[0]}-${point[1]}`;


        let cell =
        document.getElementById(id);


        if(cell &&
           id !== `${start[0]}-${start[1]}` &&
           id !== `${end[0]}-${end[1]}`)
        {
            cell.classList.add("path");
        }

    });

}

// =========================
// Remove Previous Path
// =========================
function clearPath()
{

    document.querySelectorAll(".path")
    .forEach(cell =>
    {
        cell.classList.remove("path");
    });

}

function setStatus(message, type)
{
    const status = document.getElementById("status");
    status.textContent = message;
    status.className = type;
}

document.getElementById("create-grid").addEventListener("click", createGrid);
document.getElementById("find-path").addEventListener("click", findPath);
createGrid();
