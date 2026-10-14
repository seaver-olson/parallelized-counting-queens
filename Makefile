# Makefile for the N-Queens project.
#
#   make          build ./nqueens
#   make check    run N = 4 through 12 and confirm the counts are correct
#   make clean    remove build files
#
# To add a parallel version, add a target that links your .c file with
# utils.o. For example:
#
#   nqueens_omp: nqueens_omp.c utils.o utils.h
#   	$(CC) $(CFLAGS) -fopenmp -o $@ nqueens_omp.c utils.o
#
#   nqueens_mpi: nqueens_mpi.c utils.o utils.h
#   	mpicc $(CFLAGS) -o $@ nqueens_mpi.c utils.o

CC     = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra

all: nqueens

nqueens: nqueens.c utils.o utils.h
	$(CC) $(CFLAGS) -o $@ nqueens.c utils.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

check: nqueens
	@for n in 4 5 6 7 8 9 10 11 12 13 14 15 16; do \
		./nqueens $$n | grep Solutions | sed "s/^/N=$$n  /"; \
	done

clean:
	rm -f nqueens *.o

.PHONY: all check clean
