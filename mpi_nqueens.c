/*
 * MPI N-Queens solver: distribute first-row branches across processes.
 *
 * Parallel program that counts the solutions to the N-Queens problem:
 * the number of ways to place N queens on an N x N chessboard so that no
 * two queens share a row, column, or diagonal.
 *
 * Build:  make mpi
 * Run:    mpirun -np 4 ./mpi_nqueens N
 *
 * Every solution has exactly one queen in each row, so we fill the board
 * one row at a time, top to bottom. Bitmasks track occupied columns and
 * diagonal attacks so we visit only legal squares. Passing masks by value
 * preserves the previous row's state for recursive backtracking. Mirror
 * symmetry lets us search half the first row and double its count; an odd
 * board's center column is counted once. Each process searches its assigned
 * branches serially, then MPI_Reduce sums their counts on process 0.
 *
 * Reading N, checking the answer, and printing are in utils.c; timing uses
 * MPI_Wtime(). Feel free to use any of the material
 * from utils.c in your own code, but I recommend against changing it.
 * You are also welcome to add to utils.c and to use anything from this file
 * in your own code. You may also add new files to the project.
 *
 * I especially recommend using the print function from utils.c to keep your
 * outputs consistent, but it is not requried as long as you tell me how to use
 * and interpret your output.
 *
 * Don't forget to modify the preamble of any files you touch and to build a 
 * README.
 *
 * A Makefile is provided for this assignment. You are encouraged to modify the
 * given Makefile to add new targets for your own code.
 *
 * This code was written for COMP 364/464 at Loyola University Chicago
 *
 * AI Acknowledgement:
 * This code was built with the help of AI tools, including Anthropic's Claude
 * Opus 5.5 and Qwen-25B-Flash. AI tools were used to assist in proofreading, splitting
 * functions, and structuring comments. Testing was performed manually.
 * OpenAI Codex assisted with bitmask backtracking, mirror symmetry, and MPI.
 */

#include <mpi.h>

#include "utils.h"

/* Low N bits mark the board's columns. Set once in main(). */
static unsigned all;

/*
 * Count completions using occupied columns and diagonal attacks in this row.
 */
unsigned long long solve(unsigned columns, unsigned left, unsigned right)
{
    /* One queen per row and column: all columns filled means N queens. */
    if (columns == all) {
        return 1;
    }

    unsigned long long count = 0;
    unsigned available = all & ~(columns | left | right);

    while (available) {
        /* Select the lowest legal bit, then remove it from this row's choices. */
        unsigned queen = available & (0u - available);
        available &= available - 1u;

        /* Diagonal attacks move one column sideways in the next row. */
        count += solve(columns | queen,
                       ((left | queen) << 1) & all,
                       (right | queen) >> 1);
    }

    return count;
}

int main(int argc, char *argv[])
{
    int n = parse_board_size(argc, argv);
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    all = (1u << n) - 1u;

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    unsigned long long local_count = 0;

    /* Assign columns round-robin; double mirrored branches, but not the center. */
    for (int col = rank; col < (n + 1) / 2; col += size) {
        unsigned queen = 1u << col;
        local_count += solve(queen, queen << 1, queen >> 1)
                       * (col < n / 2 ? 2 : 1);
    }

    unsigned long long count = 0;
    MPI_Reduce(&local_count, &count, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM,
               0, MPI_COMM_WORLD);
    double end = MPI_Wtime();

    if (rank == 0) {
        print_results("mpi", n, count, end - start);
    }
    MPI_Finalize();

    return 0;
}
