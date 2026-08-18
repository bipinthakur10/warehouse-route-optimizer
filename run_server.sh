#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"
cmake -S . -B build >/dev/null
cmake --build build -- -j"$(nproc)" >/dev/null
./build/server
