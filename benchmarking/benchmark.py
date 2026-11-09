#!/usr/bin/env python3

import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shlex
import signal
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parent.parent
CPUS = len(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else (os.cpu_count() or 1)
BINARIES = {"serial": "build/nqueens", "mpi": "build/mpi_nqueens",
            "openmp-static": "build/openmp_static_nqueens", "openmp-dynamic": "build/openmp_dynamic_nqueens",
            "hybrid": "build/hybrid_nqueens"}


def stop(process):
    #Terminate the MPI launcher and its local children as a process group
    for sig in (signal.SIGTERM, signal.SIGKILL):
        try:
            os.killpg(process.pid, sig)
        except ProcessLookupError:
            pass
        try:
            return process.communicate(timeout=5)
        except subprocess.TimeoutExpired:
            continue
    raise RuntimeError("Could not stop the timed-out process group")


def measure(command, threads, timeout):
    env = dict(os.environ, OMP_NUM_THREADS=str(threads), OMP_DYNAMIC="FALSE")
    start = time.perf_counter()
    process = subprocess.Popen(command, cwd=ROOT, env=env, text=True,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                               start_new_session=True)
    try:
        output, errors = process.communicate(timeout=timeout)
    except (subprocess.TimeoutExpired, KeyboardInterrupt) as exc:
        stop(process)
        if isinstance(exc, KeyboardInterrupt):
            raise
        return "exceeded walltime", "", "", time.perf_counter() - start
    wall = time.perf_counter() - start
    counts = re.findall(r"^Solutions:\s+(\d+) \(correct\)$", output, re.MULTILINE)
    times = re.findall(r"^Time:\s+([0-9.]+) seconds$", output, re.MULTILINE)
    if process.returncode or len(counts) != 1 or len(times) != 1:
        print(output + errors, file=sys.stderr)
        return "error", "", "", wall
    return "correct", int(counts[0]), float(times[0]), wall


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--n", nargs="+", type=int, default=list(range(4, 17)))
    parser.add_argument("--workers", nargs="+", type=int,
                        default=sorted({w for w in (1, 2, 4, 8, 12, 24, CPUS) if w <= CPUS}))
    parser.add_argument("--hybrid", nargs="+", metavar="RANKSxTHREADS",
                        help="Explicit hybrid combinations, e.g. 2x2 2x4 4x4")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=2400, help="Wall seconds per run (default: 40 minutes)")
    parser.add_argument("--output", type=Path, help="New CSV path; default: benchmarking/results/timings-TIMESTAMP.csv")
    args = parser.parse_args()
    if any(n < 4 or n > 16 for n in args.n):
        parser.error("Use N=4..16 for comparisons with the serial solver.")
    if args.repeats < 1 or not (0 < args.timeout < float("inf")):
        parser.error("Repeats and timeout must be positive and finite.")
    if any(w < 1 or w > CPUS for w in args.workers):
        parser.error(f"Workers must be between 1 and the {CPUS} available CPUs.")
    workers = sorted(set(args.workers))
    try:
        hybrid = [tuple(map(int, value.split("x"))) for value in args.hybrid] if args.hybrid else [
            (2, w // 2) if w % 2 == 0 else (1, w) for w in workers]
        if any(len(pair) != 2 or min(pair) < 1 or pair[0] * pair[1] > CPUS for pair in hybrid):
            raise ValueError
    except ValueError:
        parser.error(f"Hybrid combinations must be positive RANKSxTHREADS with at most {CPUS} total workers.")

    cases = [("serial", 1, 1)]
    for w in workers:
        cases.extend([("openmp-static", 1, w), ("openmp-dynamic", 1, w), ("mpi", w, 1)])
    cases.extend(("hybrid", ranks, threads) for ranks, threads in sorted(set(hybrid)))
    for name in BINARIES.values():
        if not (ROOT / name).is_file():
            parser.error("Build the executables first with 'make all'.")

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S.%fZ")
    output = args.output or ROOT / "benchmarking" / "results" / f"timings-{stamp}.csv"
    if not output.is_absolute():
        output = ROOT / output
    metadata = output.with_suffix(".meta.json")
    if output.exists() or metadata.exists():
        parser.error("Output already exists; choose a new --output path to preserve previous results.")
    output.parent.mkdir(parents=True, exist_ok=True)
    sources = list((ROOT / "nqueens").rglob("*.c")) + list((ROOT / "nqueens").rglob("*.h"))
    sources += [ROOT / "Makefile", Path(__file__).resolve()]
    with metadata.open("x") as stream:
        json.dump({"started_utc": stamp, "platform": platform.platform(), "available_cpus": CPUS,
                   "configuration": vars(args), "cases": cases,
                   "makeflags": os.environ.get("MAKEFLAGS", ""),
                   "makefile": (ROOT / "Makefile").read_text(),
                   "source_sha256": {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}},
                  stream, indent=2, default=str)

    columns = ["version", "n", "workers", "ranks", "threads", "trial", "solver_seconds",
               "wall_seconds", "solutions", "status", "command"]
    print(f"Recording {len(cases)} configurations per N, {args.repeats} trials each, to {output}", flush=True)
    with output.open("x", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=columns)
        writer.writeheader()
        for n in sorted(set(args.n)):
            for version, ranks, threads in cases:
                command = [str(ROOT / BINARIES[version]), str(n)]
                if version in ("mpi", "hybrid"):
                    command = ["mpirun", "-np", str(ranks)] + command
                for trial in range(1, args.repeats + 1):
                    status, count, seconds, wall = measure(command, threads, args.timeout)
                    writer.writerow(dict(version=version, n=n, workers=ranks * threads,
                                         ranks=ranks, threads=threads, trial=trial,
                                         solver_seconds=seconds, wall_seconds=f"{wall:.6f}",
                                         solutions=count, status=status, command=shlex.join(command)))
                    stream.flush()
                    print(f"{version:14s} N={n:2d} {ranks}x{threads} trial={trial}: {status} {seconds}", flush=True)
                    if status == "error":
                        return 1
                    if status == "exceeded walltime":
                        break  # Keep this record and move to the next configuration.
    print(f"Saved {output} and {metadata}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
