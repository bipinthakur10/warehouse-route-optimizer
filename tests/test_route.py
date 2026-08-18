import json
import subprocess
import sys
import time


def run_test(algorithm):
    payload = json.dumps({
        "rows": 5,
        "cols": 5,
        "start": [0, 0],
        "end": [4, 4],
        "obstacles": [[1, 1], [2, 2], [3, 3]],
        "algorithm": algorithm
    }).encode()

    result = subprocess.check_output([
        "curl",
        "-s",
        "-X", "POST",
        "http://127.0.0.1:8080/route",
        "-H", "Content-Type: application/json",
        "--data-binary", payload
    ], timeout=2)

    response = json.loads(result.decode())

    assert response["success"] is True
    assert response["algorithm"] == algorithm
    assert "path" in response
    assert len(response["path"]) > 0
    assert response["steps"] == max(0, len(response["path"]) - 1)

    print(f"✅ {algorithm} test passed")
   

def main():
    proc = subprocess.Popen(
        ["/home/binay/Desktop/DSA_Project/build/server"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True
    )
    try:
        # wait for server to start
        for _ in range(30):
            try:
                run_test("BFS")
                run_test("A*")
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
