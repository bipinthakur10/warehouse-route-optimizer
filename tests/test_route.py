import json
from pathlib import Path
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
SERVER = ROOT / "build" / "server"
ALGORITHMS = {"A*", "DIJKSTRA", "FLOYD-WARSHALL"}


def run_test(algorithm):
    payload = json.dumps({
        "rows": 5,
        "cols": 5,
        "start": [0, 0],
        "targets": [[0, 4], [4, 4]],
        "obstacles": [[1, 1], [2, 2], [3, 3]],
        "algorithm": algorithm,
    }).encode()

    result = subprocess.check_output([
        "curl", "-s", "-X", "POST", "http://127.0.0.1:8080/route",
        "-H", "Content-Type: application/json", "--data-binary", payload,
    ], timeout=10)
    response = json.loads(result.decode())

    assert response["success"] is True
    assert response["algorithm"] in ALGORITHMS
    assert response["steps"] == len(response["path"]) - 1
    assert len(response["targetPaths"]) == 2
    assert response["targetPaths"][0][0] == [0, 0]
    assert response["targetPaths"][1][-1] == [4, 4]

    comparisons = response["comparisons"]
    assert {item["algorithm"] for item in comparisons} == ALGORITHMS
    assert all(item["success"] for item in comparisons)
    assert len({item["cost"] for item in comparisons}) == 1
    print(f"{algorithm} comparison test passed")


def main():
    proc = subprocess.Popen([str(SERVER)], cwd=ROOT)
    try:
        for _ in range(30):
            try:
                for algorithm in ("A*", "Dijkstra", "Floyd-Warshall"):
                    run_test(algorithm)
                return 0
            except Exception:
                time.sleep(0.2)
        print("route endpoint test failed")
        return 1
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()


if __name__ == "__main__":
    sys.exit(main())
