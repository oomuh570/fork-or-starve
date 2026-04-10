/*
 * File: display.c
 * Purpose: Prints each philosopher's current state to the terminal.
 */

#include <stdio.h>
#include "philosopher.h"

#define GREEN "\033[92m"   // EATING
#define RED "\033[91m"     // HUNGRY
#define BLUE "\033[94m"    // THINKING
#define RESET "\033[0m"

/*
 * Function: print_state
 * Purpose: Prints one philosopher's current state with a color.
 * Params:
 *   i     - philosopher number
 *   state - current state of the philosopher
 * Returns: void
 */
void print_state(long i, int state) {
    if (state == THINKING) {
        printf("%sP%ld %s is THINKING%s\n", BLUE, i, phil_names[i], RESET);
    }
    else if (state == HUNGRY) {
        printf("%sP%ld %s is HUNGRY%s\n", RED, i, phil_names[i], RESET);
    }
    else if (state == EATING) {
        printf("%sP%ld %s is EATING%s\n", GREEN, i, phil_names[i], RESET);
    }
}
