/*
 * File: stats.c
 * Purpose: Tracks meals eaten and checks for starvation.
 */

#include <stdio.h>
#include "philosopher.h"

#define YELLOW "\033[93m"
#define MAGENTA "\033[95m"
#define TEAL "\033[38;5;37m"
#define DARK_RED "\033[38;5;88m"
#define RESET "\033[0m"

/*
 * Function: meal_record
 * Purpose: Adds one meal to a philosopher's total and prints it.
 * Params:
 *   i - philosopher number
 * Returns: void
 */
void meal_record(long i) {
    meals[i]++;
    printf("%sP%ld %s have eaten %d meal(s)%s\n", YELLOW, i, phil_names[i], meals[i], RESET);
}

/*
 * Function: starve_check
 * Purpose: Checks meal counts to see if starvation may be happening.
 * Params:
 *   count - current step count used for checking starvation
 * Returns: void
 */
void starve_check(int count) {
    int min = meals[0];
    int max = meals[0];
    int min_index = 0;

    for (int i = 1; i < NUM_PHILS; i++) {
        if (meals[i] < min) {
            min = meals[i];
            min_index = i;
        }
        if (meals[i] > max) {
            max = meals[i];
        }
    }

    if (count >= 50 && min == 0) {
        printf("%sFull starvation detected: P%d %s has 0 meals after %d steps%s\n",
            DARK_RED, min_index, phil_names[min_index], count, RESET);
    }
    else if (max - min > 3) {
        printf("%sPotential starvation detected: P%d %s is lagging behind%s\n",
            MAGENTA, min_index, phil_names[min_index], RESET);
    }
    else {
        printf("%sFair distribution%s\n", TEAL, RESET);
    }
}
