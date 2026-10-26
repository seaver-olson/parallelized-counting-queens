/*
 * OpenMP dynamic N-Queens solver: distribute placements in the first two rows.
 *
 * Parallel program that counts the solutions to the N-Queens problem:
 * the number of ways to place N queens on an N x N chessboard so that no
 * two queens share a row, column, or diagonal.
 *
 * Build:  make openmp-dynamic
 * Run:    OMP_NUM_THREADS=4 ./openmp_dynamic_nqueens N
 *
 * Every solution has exactly one queen in each row, so we fill the board
 * one row at a time, top to bottom. Bitmasks track unused columns and
 * diagonal attacks so we visit only legal squares. Passing masks by value
 * preserves the previous row's state for recursive backtracking. Mirror
 * symmetry lets us search half the first row and double its count; an odd
 * board's center column is counted once. OpenMP distributes legal placements
 * in the first two rows across threads and sums counts with a reduction.
 *
 * Reading N, checking the answer, and printing are in utils.c; timing uses
 * omp_get_wtime(). Feel free to use any of the material
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
 * OpenAI Codex assisted with bitmask backtracking, mirror symmetry, and OpenMP.
 */

#include <omp.h>

#include "utils.h"

/* Low N bits mark the board's columns. Set once in main(). */
static unsigned all;

/*
 * Count completions using unused columns and diagonal attacks in this row.
 * remaining is nonzero; the final queen is counted without recursing.
 */
unsigned long long solve(unsigned remaining, unsigned left, unsigned right)
{
    unsigned available = remaining & ~(left | right);
    /* With one unused column, count the final row without another call. */
    if (!(remaining & (remaining - 1u))) {
        return available != 0;
    }
    /* A next row with no possible square cannot be rescued by this row. */
    if (!(remaining & ~((left << 1) | (right >> 1)))) return 0;

    unsigned long long count = 0;
    while (available) {
        /* Select the lowest legal bit, then remove it from this row's choices. */
        unsigned queen = available & (0u - available);
        available &= available - 1u;

        /* Diagonal attacks move one column sideways in the next row. */
        count += solve(remaining ^ queen, (left | queen) << 1,
                       (right | queen) >> 1);
    }

    return count;
}

int main(int argc, char *argv[])
{
    int n = parse_board_size(argc, argv);
    all = (1u << n) - 1u;

    double start = omp_get_wtime();
    unsigned long long count = 0;

    /* Task / n is the first column; task % n is the second row's column. */
    #pragma omp parallel for reduction(+:count) schedule(dynamic, 1)
    for (int task = 0; task < ((n + 1) / 2) * n; task++) {
        int col = task / n;
        unsigned first = 1u << col, queen = 1u << (task % n);
        if (queen & (first | (first << 1) | (first >> 1))) continue;
        count += solve(all ^ (first | queen), (((first << 1) | queen) << 1) & all,
                       ((first >> 1) | queen) >> 1)
                 * (col < n / 2 ? 2 : 1);
    }

    double end = omp_get_wtime();

    print_results("omp-dynamic", n, count, end - start);

    return 0;
}
