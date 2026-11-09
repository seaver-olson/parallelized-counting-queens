#!/usr/bin/env bash
# One perf file per process. Pick a different PERF_DIR for each run.
set -e
rank=${OMPI_COMM_WORLD_RANK:-${PMI_RANK:-0}}
dir=${PERF_DIR:-results/perf/run}
mkdir -p "$dir"
export LC_ALL=C
exec perf stat --no-big-num -e \
    task-clock,cycles:u,instructions:u,branches:u,branch-misses:u,cache-references:u,cache-misses:u,context-switches,cpu-migrations \
    -o "$dir/perf_worker_$rank.txt" -- "$@"
