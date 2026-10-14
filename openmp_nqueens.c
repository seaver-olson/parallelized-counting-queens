/*
 * nqueens.c
 *
 * Serial program that counts the solutions to the N-Queens problem:
 * the number of ways to place N queens on an N x N chessboard so that no
 * two queens share a row, column, or diagonal.
 *
 * Build:  make
 * Run:    ./nqueens N        (for example: ./nqueens 10)
 *
 * Every solution has exactly one queen in each row, so we fill the board
 * one row at a time, top to bottom. In each row we try every column. If a
 * square is not attacked by any queen already on the board, we place a
 * queen there and move on to the next row. When the last row is filled we
 * have found one solution. Afterward we remove the queen and try the next
 * column. This is called recursive backtracking.
 *
 * Everything that is not part of the search (reading N, timing, checking
 * the answer, printing) is in utils.c. Feel free to use any of the material
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
 */

#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"

/* Board size set in main(). */
int n;

/*
 * The board array sa a 1D array of ints 
 * queens[row] is the column of the queen in that row.
 * Only rows 0 through (current row - 1) contain valid entries during the search.
 */
int queens[MAX_N];

/*
 * Return true if a queen at (row, col) is not attacked by any queen
 * in the rows above it.
 */
bool is_safe(int row, int col)
{
    for (int r = 0; r < row; r++) {
        int c = queens[r];

        /* Same column. */
        if (c == col) {
            return false;
        }

        /* Same diagonal: the row distance equals the column distance. */
        if (abs(c - col) == row - r) {
            return false;
        }
    }
    return true;
}

/*
 * Count all solutions that can be reached from the current board, given
 * that rows 0 through row-1 already have a queen.
 */
unsigned long long solve(int row)
{
    /* All N rows have a queen */
    if (row == n) {
        return 1;
    }

    unsigned long long count = 0;

    for (int col = 0; col < n; col++) {
        if (is_safe(row, col)) {
            queens[row] = col;           /* place a queen */
            count += solve(row + 1);     /* fill the remaining rows */
        }
    }

    return count;
}

int main(int argc, char *argv[])
{
    n = parse_board_size(argc, argv);

    double start = get_time();
    unsigned long long count = solve(0);
    double end = get_time();

    print_results("serial", n, count, end - start);

    return 0;
}
