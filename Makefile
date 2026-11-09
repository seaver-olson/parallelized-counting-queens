# Makefile for the N-Queens project.
#   make / make check : build / test all versions (N = 4..16)
#   make serial / check-serial : build / test the matching serial baseline
#   make mpi / openmp / hybrid : build one version
#   make check-mpi / check-openmp / check-hybrid : test one version
#   make check-mpi N="16 17" WORKERS=4 : choose cases and parallel workers
#   make check-hybrid N=17 WORKERS=2 THREADS=4 : processes and threads/process
#   make openmp-static / openmp-dynamic (or check-openmp-static / check-openmp-dynamic)
#   make clean : remove build files
#   make benchmark N="4 8 12 14 16"  save timing sweeps as CSV

CC     = gcc
MPICC  ?= mpicc
CFLAGS = -std=c11 -O3 -g -Wall -Wextra
N      ?= 4 5 6 7 8 9 10 11 12 13 14 15 16
WORKERS ?= 4
THREADS ?= 4

all: serial mpi openmp hybrid

serial: build/nqueens
mpi: build/mpi_nqueens
openmp: openmp-static openmp-dynamic
openmp-static: build/openmp_static_nqueens
openmp-dynamic: build/openmp_dynamic_nqueens
hybrid: build/hybrid_nqueens

build/nqueens: nqueens_serial/nqueens.c build/utils.o utils.h solve.h | build
	$(CC) $(CFLAGS) -o $@ $< build/utils.o

build/mpi_nqueens: mpi_nqueens.c build/utils.o utils.h solve.h | build
	$(MPICC) $(CFLAGS) -o $@ $< build/utils.o

build/openmp_static_nqueens build/openmp_dynamic_nqueens: build/%: %.c build/utils.o utils.h solve.h | build
	$(CC) $(CFLAGS) -fopenmp -o $@ $< build/utils.o

build/hybrid_nqueens: hybrid_nqueens.c build/utils.o utils.h solve.h | build
	$(MPICC) $(CFLAGS) -fopenmp -o $@ $< build/utils.o

build/utils.o: utils.c utils.h | build
	$(CC) $(CFLAGS) -c -o $@ $<

build:
	mkdir -p $@

check: check-serial check-mpi check-openmp check-hybrid
check-openmp: check-openmp-static check-openmp-dynamic

check-mpi: RUN = mpirun -np $(WORKERS)
check-openmp-static check-openmp-dynamic: RUN = env OMP_NUM_THREADS=$(WORKERS) OMP_DYNAMIC=FALSE
check-hybrid: RUN = env OMP_NUM_THREADS=$(THREADS) OMP_DYNAMIC=FALSE mpirun -np $(WORKERS)

check-serial: build/nqueens
check-mpi: build/mpi_nqueens
check-openmp-static: build/openmp_static_nqueens
check-openmp-dynamic: build/openmp_dynamic_nqueens
check-hybrid: build/hybrid_nqueens

check-serial check-mpi check-openmp-static check-openmp-dynamic check-hybrid:
	@for n in $(N); do \ 
		result=$$($(RUN) ./$< $$n) || exit 1; \
		printf '%s\n' "$$result" | grep -E 'Solutions|Time' | sed "s/^/$(patsubst check-%,%,$@) N=$$n  /"; \
		printf '%s\n' "$$result" | grep -q '(correct)' || exit 1; \
	done

benchmark: all
	python3 benchmark.py --n $(N) $(BENCH_ARGS)

clean:
	rm -rf build

.PHONY: all serial mpi openmp openmp-static openmp-dynamic hybrid check check-serial check-mpi check-openmp check-openmp-static check-openmp-dynamic check-hybrid benchmark clean
