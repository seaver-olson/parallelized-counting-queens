import argparse
import csv
from collections import defaultdict
from math import isfinite
from pathlib import Path
from statistics import median

import matplotlib
matplotlib.use("Agg")  # Save files without needing a desktop/window.

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("csv", type=Path, help="Result file written by benchmark.py")
args = parser.parse_args()

# Keep different hybrid rank/thread combinations separate when averaging trials.
trials = defaultdict(list)
with args.csv.open(newline="") as stream:
    for row in csv.DictReader(stream):
        if row["status"] != "correct":
            print(f"Skipping N={row['n']} {row['version']}: {row['status']}")
            continue
        seconds = float(row["solver_seconds"])
        if not isfinite(seconds) or seconds < 0:
            parser.error("Solver times must be finite and nonnegative.")
        key = (int(row["n"]), row["version"], int(row["ranks"]), int(row["threads"]))
        trials[key].append(seconds)

times = {key: median(values) for key, values in trials.items()}
for key in list(times):
    if times[key] == 0:
        print(f"Skipping {key}: median rounded to zero; speedup is undefined.")
        del times[key]
boards = sorted({n for n, version, ranks, threads in times
                 if version == "serial" and ranks == threads == 1})
if not boards:
    parser.error("No positive serial medians available for comparison.")

output = args.csv.parent / (args.csv.stem + "-plots")
output.mkdir(exist_ok=True)
names = {"mpi": "MPI", "openmp-static": "OpenMP static",
         "openmp-dynamic": "OpenMP dynamic", "hybrid": "Hybrid"}
for n in boards:
    serial = times[n, "serial", 1, 1]
    fig, (runtime, speedup) = matplotlib.pyplot.subplots(1, 2, figsize=(11, 5))
    series = sorted({(v, r if v == "hybrid" else 0)
                     for size, v, r, t in times if size == n and v != "serial"})
    for version, rank_group in series:
        points = sorted((r * t, seconds) for (size, v, r, t), seconds in times.items()
                        if size == n and v == version and (not rank_group or r == rank_group))
        workers, seconds = zip(*points)
        label = names[version] + (f" (MPI ranks: {rank_group})" if rank_group else "")
        runtime.plot(workers, seconds, "o-", label=label)
        speedup.plot(workers, [serial / value for value in seconds], "o-", label=label)

    runtime.axhline(serial, color="black", linestyle="--", label="Serial baseline")
    runtime.set(title="Runtime", ylabel="Median solver seconds (log scale)", yscale="log")
    speedup.axhline(1, color="black", linestyle="--")
    speedup.set(title="Speedup vs serial", ylabel="Serial time / parallel time", ylim=(0, None))
    ticks = sorted({r * t for size, v, r, t in times if size == n})
    for ax in (runtime, speedup):
        ax.set_xlabel("Workers (MPI ranks x OpenMP threads)")
        ax.set_xticks(ticks)
        ax.grid(True, alpha=0.25)
    fig.suptitle(f"N-Queens: N = {n} (median across trials)")
    fig.legend(*runtime.get_legend_handles_labels(), loc="lower center",
               bbox_to_anchor=(0.5, 0.045), ncol=3, fontsize=9)
    fig.text(0.5, 0.015, f"Source: {args.csv.name} | Baseline: serial rows in this CSV",
             ha="center", fontsize=8)
    fig.subplots_adjust(bottom=0.29, top=0.86, wspace=0.3)
    for extension in ("png", "pdf"):
        path = output / f"n{n}.{extension}"
        fig.savefig(path, dpi=180)
        print(path)
    matplotlib.pyplot.close(fig)
