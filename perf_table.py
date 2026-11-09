#Turn perf_worker.sh output files into a CSV table
import csv
from collections import defaultdict
from pathlib import Path
from statistics import median
import sys
from re import search

UNITS = {"nsec": 1e-6, "usec": 1e-3, "msec": 1, "sec": 1000}#lookup table for time units

#pull 1 counter val out of perf report
def counter(text, name):
    for line in text.splitlines():
        parts = line.split()
        if len(parts) < 2:
            continue
        try:
            val = float(parts[0])
        except ValueError:
            continue
        rest = parts[1:]
        unit = None
        if rest[0] in UNITS:
            unit, rest = rest[0], rest[1:]
        if rest and rest[0] in (name, name + ":u"):
            return val, unit
    return None, None


def ratio(top, bottom, scale=1):
    return scale * top / bottom if top is not None and bottom else None


def show(value, digits=3):
    return "N/A" if value is None else f"{value:.{digits}f}"


median_mode = "--median" in sys.argv[1:]
filenames = [name for name in sys.argv[1:] if name != "--median"]
if not filenames:
    sys.exit("Usage: python3 perf_table.py results/perf/*/*.txt > results/perf/table.csv")

table = csv.writer(sys.stdout)
table.writerow(["configuration/worker" if median_mode else "file",
                "wall_seconds", "task_clock_ms", "cpus_utilized",
                "instructions", "ipc", "branch_miss_pct", "cache_miss_pct"])
groups = defaultdict(list)
for filename in filenames:
    text = Path(filename).read_text()
    elapsed = search(r"([\d.]+) seconds time elapsed", text)
    if not elapsed:
        sys.exit(f"No elapsed time in {filename}; check that perf finished successfully.")
    wall = float(elapsed[1])
    clock, unit = counter(text, "task-clock")
    # An unscaled task-clock count is in nanoseconds. Older perf prints msec.
    clock_ms = clock * {None: 1e-6, "nsec": 1e-6, "usec": 1e-3,
                        "msec": 1, "sec": 1000}[unit] if clock is not None else None
    cycles = counter(text, "cycles")[0]
    instructions = counter(text, "instructions")[0]
    branches = counter(text, "branches")[0]
    branch_misses = counter(text, "branch-misses")[0]
    references = counter(text, "cache-references")[0]
    cache_misses = counter(text, "cache-misses")[0]
    values = [wall, clock_ms, ratio(clock_ms, wall * 1000), instructions,
              ratio(instructions, cycles), ratio(branch_misses, branches, 100),
              ratio(cache_misses, references, 100)]
    if median_mode:
        path = Path(filename)
        groups[path.parents[2].name, path.parent.parent.name, path.name].append(values)
    else:
        table.writerow([filename, show(wall, 6), *[show(value, 0 if i == 2 else 3)
                                                   for i, value in enumerate(values[1:])]])

if median_mode:
    for run, configuration, worker in sorted(groups):
        trials = groups[run, configuration, worker]
        if len(trials) != 3:
            sys.exit(f"{run}/{configuration}/{worker} has {len(trials)} trials; expected 3.")
        values = [median(column) if all(value is not None for value in column) else None
                  for column in zip(*trials)]
        table.writerow([f"{run}/{configuration}/{worker}", show(values[0], 6),
                        *[show(value, 0 if i == 2 else 3)
                          for i, value in enumerate(values[1:])]])
