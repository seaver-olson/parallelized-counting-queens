/*
 * utils.h
 *
 * Helper functions for the N-Queens project: reading N from the command
 * line, timing, checking answers, and printing results.
 * Accepted board sizes extend through N = 27, with known solution counts.
 *
 * You should not need to change this file or utils.c. Include this header
 * in each of your parallel versions and link against utils.o (see the
 * Makefile). You are welcome to add to this file, however.
 * 
 * This code was written for COMP 364/464 at Loyola University Chicago
 *
 * AI Acknowledgement:
 * This code was built with the help of AI tools, including Anthropic's Claude
 * Opus 5.5 and Qwen-25B-Flash. AI tools were used to assist in proofreading, splitting
 * functions, and structuring comments. Testing was performed manually.
 */

/* Smallest and largest board sizes the program accepts. */
#define MIN_N 4
#define MAX_N 27

/*
 * Read the board size N from argv[1].
 * Prints a usage message and exits if N is missing or out of range.
 */
int parse_board_size(int argc, char *argv[]);

/*
 * Return the current wall-clock time in seconds.
 * Only differences between two calls to this function are meaningful.
 *
 * In your MPI and hybrid versions, use MPI_Wtime() instead.
 * In your OpenMP versions, you may use omp_get_wtime() or this function.
 */
double get_time(void);

/*
 * Return the known number of solutions for an N x N board,
 * or -1 if N is outside the table.
 */
long long expected_solutions(int n);

/*
 * Print the board size, solution count, whether the count is correct,
 * and the elapsed time.
 *
 * version: a short label for the implementation, e.g. "serial",
 *          "omp-static", "mpi", "hybrid".
 */
void print_results(const char *version, int n, unsigned long long count,
                   double seconds);
