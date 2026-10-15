# Makefile for the N-Queens project.
#
#   make / make check                 build / test all versions (N = 4..16)
#   make mpi / openmp / hybrid        build one version
#   make check-mpi / check-openmp / check-hybrid   test one version
#   make check-mpi N="16 17" WORKERS=4  choose cases and parallel workers
#   make check-hybrid N=17 WORKERS=2 THREADS=4   processes and threads/process
#   make openmp-static / openmp-dynamic (or check-openmp-static / check-openmp-dynamic)
#   make clean                        remove build files

CC     = gcc
MPICC  ?= mpicc
CFLAGS = -std=c11 -O2 -Wall -Wextra
N      ?= 4 5 6 7 8 9 10 11 12 13 14 15 16
WORKERS ?= 4
THREADS ?= 4

all: mpi openmp hybrid

mpi: mpi_nqueens
openmp: openmp-static openmp-dynamic
openmp-static: openmp_static_nqueens
openmp-dynamic: openmp_dynamic_nqueens
hybrid: hybrid_nqueens

mpi_nqueens: mpi_nqueens.c utils.o utils.h
	$(MPICC) $(CFLAGS) -o $@ $< utils.o

openmp_static_nqueens openmp_dynamic_nqueens: %: %.c utils.o utils.h
	$(CC) $(CFLAGS) -fopenmp -o $@ $< utils.o

hybrid_nqueens: hybrid_nqueens.c utils.o utils.h
	$(MPICC) $(CFLAGS) -fopenmp -o $@ $< utils.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

check: check-mpi check-openmp check-hybrid
check-openmp: check-openmp-static check-openmp-dynamic

check-mpi: RUN = mpirun -np $(WORKERS)
check-openmp-static check-openmp-dynamic: RUN = env OMP_NUM_THREADS=$(WORKERS) OMP_DYNAMIC=FALSE
check-hybrid: RUN = env OMP_NUM_THREADS=$(THREADS) OMP_DYNAMIC=FALSE mpirun -np $(WORKERS)

check-mpi: mpi_nqueens
check-openmp-static: openmp_static_nqueens
check-openmp-dynamic: openmp_dynamic_nqueens
check-hybrid: hybrid_nqueens

check-mpi check-openmp-static check-openmp-dynamic check-hybrid:
	@for n in $(N); do \
		result=$$($(RUN) ./$< $$n) || exit 1; \
		printf '%s\n' "$$result" | grep -E 'Solutions|Time' | sed "s/^/$(patsubst check-%,%,$@) N=$$n  /"; \
		printf '%s\n' "$$result" | grep -q '(correct)' || exit 1; \
	done

clean:
	rm -f nqueens mpi_nqueens openmp_nqueens openmp_static_nqueens openmp_dynamic_nqueens hybrid_nqueens *.o

.PHONY: all mpi openmp openmp-static openmp-dynamic hybrid check check-mpi check-openmp check-openmp-static check-openmp-dynamic check-hybrid clean
