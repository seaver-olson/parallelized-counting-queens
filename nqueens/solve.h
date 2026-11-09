// Shared N-Queens search for the serial and parallel programs.
#ifndef NQUEENS_SOLVE_H
#define NQUEENS_SOLVE_H

unsigned long long solve(unsigned remaining, unsigned left, unsigned right)
{
    unsigned available = remaining & ~(left | right);
    // With one unused column, count the final row without another call
    if (!(remaining & (remaining - 1u))) {
        return available != 0;
    }
    // A next row with no possible square cannot be rescued by this row
    if (!(remaining & ~((left << 1) | (right >> 1)))) return 0;

    unsigned long long count = 0;
    while (available) {
        // Select the lowest legal bit, then remove it from this row's choices.
        unsigned queen = available & (0u - available);
        available &= available - 1u;

        // Diagonal attacks move one column sideways in the next row 
        count += solve(remaining ^ queen, (left | queen) << 1,
                       (right | queen) >> 1);
    }

    return count;
}

#endif
