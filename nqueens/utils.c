/*
 * utils.c
 *
 * Implementations of the helper functions declared in utils.h.
 * Known solution counts extend through N = 27 (OEIS A000170).
 *
 * You should not need to modify this file. You are welcome to add to it, however.
 * 
 * This code was written for COMP 364/464 at Loyola University Chicago
 *
 * AI Acknowledgement:
 * This code was built with the help of AI tools, including Anthropic's Claude
 * Opus 5.5 and Qwen-25B-Flash. AI tools were used to assist in proofreading, splitting
 * functions, and structuring comments. Testing was performed manually.
 */

/* Needed for clock_gettime() when compiling with -std=c11. */
#define _POSIX_C_SOURCE 199309L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"

/*
 * Read the board size N from argv[1].
 * Prints a usage message and exits if N is missing or out of range.
 */
int parse_board_size(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        fprintf(stderr, "  N = board size, between %d and %d\n", MIN_N, MAX_N);
        exit(EXIT_FAILURE);
    }

    /* strtol lets us detect input that is not a whole number, like "8x". */
    char *end;
    errno = 0;
    long n = strtol(argv[1], &end, 10);

    if (errno != 0 || *end != '\0' || end == argv[1]) {
        fprintf(stderr, "Error: '%s' is not a valid integer.\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    if (n < MIN_N || n > MAX_N) {
        fprintf(stderr, "Error: N must be between %d and %d.\n", MIN_N, MAX_N);
        exit(EXIT_FAILURE);
    }

    return (int)n;
}

/*
 * Return the current wall-clock time in seconds.
 * Only differences between two calls to this function are meaningful.
 *
 * In your MPI and hybrid versions, use MPI_Wtime() instead.
 * In your OpenMP versions, you may use omp_get_wtime() or this function.
 */
double get_time(void)
{
    /* CLOCK_MONOTONIC is not affected by changes to the system clock. */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/*
 * Return the known number of solutions for an N x N board,
 * or -1 if N is outside the table.
 */
long long expected_solutions(int n)
{
    /*
     * Known solution counts, indexed by N
     * Index 0 is unused.
     * Source: https://oeis.org/A000170/b000170.txt
     */
    static const long long known[] = {
        0,              /* N = 0, unused */
        1,              /* N = 1 */
        0,              /* N = 2 */
        0,              /* N = 3 */
        2,              /* N = 4 */
        10,             /* N = 5 */
        4,              /* N = 6 */
        40,             /* N = 7 */
        92,             /* N = 8 */
        352,            /* N = 9 */
        724,            /* N = 10 */
        2680,           /* N = 11 */
        14200,          /* N = 12 */
        73712,          /* N = 13 */
        365596,         /* N = 14 */
        2279184,        /* N = 15 */
        14772512,       /* N = 16 */
        95815104,       /* N = 17 */
        666090624,      /* N = 18 */
        4968057848LL,   /* N = 19 */
        39029188884LL,  /* N = 20 */
        314666222712LL, /* N = 21 */
        2691008701644LL, /* N = 22 */
        24233937684440LL, /* N = 23 */
        227514171973736LL, /* N = 24 */
        2207893435808352LL, /* N = 25 */
        22317699616364044LL, /* N = 26 */
        234907967154122528LL, /* N = 27 */
    };
    int table_size = (int)(sizeof(known) / sizeof(known[0]));

    if (n < 1 || n >= table_size) {
        return -1;
    }
    return known[n];
}

/*
 * Print the board size, solution count, whether the count is correct,
 * and the elapsed time.
 *
 * version: a short string label for the implementation, e.g. "serial",
 *          "omp-static", "mpi", "hybrid".
 */
void print_results(const char *version, int n, unsigned long long count,
                   double seconds)
{
    long long expected = expected_solutions(n);
    const char *status;

    if (expected < 0) {
        status = "unknown";
    } else if (count == (unsigned long long)expected) {
        status = "correct";
    } else {
        status = "WRONG";
    }

    printf("Version:   %s\n", version);
    printf("N:         %d\n", n);
    printf("Solutions: %llu (%s", count, status);
    if (expected >= 0 && count != (unsigned long long)expected) {
        printf(", expected %lld", expected);
    }
    printf(")\n");
    printf("Time:      %.6f seconds\n", seconds);
}
