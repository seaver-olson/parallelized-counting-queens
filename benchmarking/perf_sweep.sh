#!/usr/bin/env bash
# Collect three perf runs for serial, both OpenMP schedules, and MPI.  
set -euo pipefail
repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
cd "$repo_root"
n=${1:-18}
if ! [[ "$n" =~ ^[0-9]+$ ]] || (( n < 4 || n > 27 )); then
    echo "Usage: $0 [N], with N from 4 through 27" >&2
    exit 2
fi
out="benchmarking/results/perf/part1-n$n"
if [[ -e "$out" ]]; then
    echo "$out already exists; move it before collecting another sweep." >&2
    exit 1
fi
mkdir -p "$out"

run() {
    local config=$1 trial=$2
    shift 2
    local dir="$out/$config/trial-$trial"
    mkdir -p "$dir"
    echo "$config trial $trial"
    PERF_DIR="$dir" "$@" >"$dir/program.txt"
    if ! grep -q '(correct)' "$dir/program.txt"; then
        cat "$dir/program.txt" >&2
        echo "Incorrect or missing solution count in $dir/program.txt" >&2
        return 1
    fi
}

for trial in 1 2 3; do
    run serial-w1 "$trial" ./benchmarking/perf_worker.sh ./build/nqueens "$n"
    for workers in 1 2 4 8 12 24; do
        run "openmp-static-w$workers" "$trial" env OMP_NUM_THREADS="$workers" OMP_DYNAMIC=FALSE ./benchmarking/perf_worker.sh ./build/openmp_static_nqueens "$n"
        run "openmp-dynamic-w$workers" "$trial" env OMP_NUM_THREADS="$workers" OMP_DYNAMIC=FALSE ./benchmarking/perf_worker.sh ./build/openmp_dynamic_nqueens "$n"
        run "mpi-w$workers" "$trial" mpirun -np "$workers" ./benchmarking/perf_worker.sh ./build/mpi_nqueens "$n"
    done
done

echo "Raw perf files and program output: $out"
