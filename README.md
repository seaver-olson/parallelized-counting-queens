# Build

`make` builds all versions; `make mpi`, `make openmp`, and `make hybrid` build individually.
`make openmp` builds both schedules; `make openmp-static` and `make openmp-dynamic` select one.
`make clean` removes build files. MPI and hybrid builds require `mpicc`.

# Run Tests

```sh
make check                               # All versions, N = 4 through 16
make check-mpi N=17 WORKERS=4              # Four MPI processes
make check-openmp N="16 17" WORKERS=4      # Both OpenMP schedules
make check-openmp-static N=17 WORKERS=4    # Static schedule only
make check-openmp-dynamic N=17 WORKERS=4   # Dynamic schedule only
make check-hybrid N=17 WORKERS=2 THREADS=4 # Two processes, four threads each
```

All check targets accept a single `N` or a quoted list. Valid sizes are 4 through 27;
large boards take rapidly increasing time. Expected counts come from
[OEIS A000170](https://oeis.org/A000170/b000170.txt).

`WORKERS` defaults to 4: MPI/hybrid processes or OpenMP threads. For hybrid,
`THREADS` defaults to 4 threads per process, so total workers are `WORKERS * THREADS`.
Compare the printed `Time` values with equal total worker counts; MPI process
startup is outside the printed timer.

MPI and OpenMP distribute legal placements in the first two rows. Static scheduling
assigns tasks in advance; dynamic scheduling hands them out as threads finish. Hybrid assigns
first-row branches to MPI processes and second-row placements to OpenMP threads.
All four versions use the same recursive bitmask solver and mirror symmetry.
The solver counts the final row directly and prunes branches when the next row
already has no possible square. Builds use portable `-O3` optimization.

# Report
