# Build and run

Requires GCC with OpenMP, MPI (`mpicc`, `mpirun`), and Python 3.
Run the commands below from the project root.
Solver sources are in `nqueens/`; benchmark scripts and saved output are in
`benchmarking/`. Executables and object files go in `build/`; `make clean`
removes that folder.

The serial reference implementation is in `nqueens/serial/`; MPI, OpenMP,
and hybrid implementations plus shared helpers are directly under `nqueens/`.

```sh
make all
make check                                      # All versions, N=4..16
make check N="4 8 12" WORKERS=4 THREADS=2          # Short test
make benchmark N="4 8 12 14 16" BENCH_ARGS="--output benchmarking/results/comparison.csv"
```

Choose a new CSV filename for each benchmark run.

# Plot results

```sh
python3 -m venv .venv                            # One-time setup
.venv/bin/python -m pip install matplotlib
.venv/bin/python benchmarking/plot_results.py benchmarking/results/comparison.csv
```

Replace the CSV path to plot another run. Runtime and speedup graphs are saved
as PNG and PDF files in `benchmarking/results/comparison-plots/`.

# Perf counters

```sh
PERF_DIR=benchmarking/results/perf/serial-n16 ./benchmarking/perf_worker.sh ./build/nqueens 16
PERF_DIR=benchmarking/results/perf/static-n16-t4 OMP_NUM_THREADS=4 OMP_DYNAMIC=FALSE ./benchmarking/perf_worker.sh ./build/openmp_static_nqueens 16
PERF_DIR=benchmarking/results/perf/dynamic-n16-t4 OMP_NUM_THREADS=4 OMP_DYNAMIC=FALSE ./benchmarking/perf_worker.sh ./build/openmp_dynamic_nqueens 16
PERF_DIR=benchmarking/results/perf/mpi-n16-p4 mpirun -np 4 ./benchmarking/perf_worker.sh ./build/mpi_nqueens 16
PERF_DIR=benchmarking/results/perf/hybrid-n16-p2-t2 OMP_NUM_THREADS=2 OMP_DYNAMIC=FALSE mpirun -np 2 ./benchmarking/perf_worker.sh ./build/hybrid_nqueens 16
python3 benchmarking/perf_table.py benchmarking/results/perf/*/perf_worker_*.txt > benchmarking/results/perf/table.csv
```

Choose a new `PERF_DIR` for another run. MPI/hybrid get one file and table row per
rank. Wall time covers the whole worker process, including MPI setup and waiting.
Unsupported counters and undefined ratios appear as `N/A`, not zero.

For the Part 1 sweep, choose an N where serial takes at least 30 seconds (18 is
the default; confirm it on your system), then run:

```sh
./benchmarking/perf_sweep.sh 18
python3 benchmarking/perf_table.py --median benchmarking/results/perf/part1-n18/*/trial-*/perf_worker_*.txt > benchmarking/results/perf/part1-n18/medians.csv
```

The sweep records three trials at 1, 2, 4, 8, 12, and 24 workers. Serial is run
once per trial; OpenMP static/dynamic and MPI use every worker count. Raw perf
files and each run's solution check are kept under the matching results folder.
Use the same N in both commands and in the median command's input and output paths.
