/*
 * File: stats.c
 * Purpose: Tracks meals eaten and checks for starvation.
 */

#include <stdio.h>
#include <string.h>
#include "philosopher.h"

#define YELLOW   "\033[93m"
#define MAGENTA  "\033[95m"
#define TEAL     "\033[38;5;37m"
#define DARK_RED "\033[38;5;88m"
#define GREEN    "\033[92m"
#define CYAN     "\033[96m"
#define BOLD     "\033[1m"
#define RESET    "\033[0m"

/*
 * Function: meal_record
 * Purpose: Adds one meal to a philosopher's total and prints it.
 * Params:
 *   i - philosopher number
 * Returns: void
 */
void meal_record(long i) {
    meals[i]++;
    //printf("%sP%ld %s have eaten %d meal(s)%s\n", YELLOW, i, phil_names[i], meals[i], RESET);
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
        //printf("%sFull starvation detected: P%d %s has 0 meals after %d steps%s\n",
            //DARK_RED, min_index, phil_names[min_index], count, RESET);
    }
    else if (max - min > 3) {
        //printf("%sPotential starvation detected: P%d %s is lagging behind%s\n",
            //MAGENTA, min_index, phil_names[min_index], RESET);
    }
    else {
        //printf("%sFair distribution%s\n", TEAL, RESET);
    }
}

/*
 * Function: print_stats
 * Purpose:  Prints final statistics summary when simulation ends.
 *           Shows mode, total cycles, meals per philosopher,
 *           and overall fairness assessment.
 * Returns:  void
 */
void print_stats() {
    char *mode_str;
    if      (mode == 0) mode_str = "NAIVE (mode=0)";
    else if (mode == 1) mode_str = "ASYMMETRIC (mode=1)";
    else                mode_str = "WAITER (mode=2)";

    /* calculate total meals */
    int total = 0;
    for (int i = 0; i < NUM_PHILS; i++)
        total += meals[i];

    /* calculate average */
    int avg = (total > 0) ? total / NUM_PHILS : 0;

    printf(BOLD CYAN "\n  ── SIMULATION COMPLETE ─────────────────────────────────\n" RESET);
    printf("  Mode              : %s\n", mode_str);
    printf("  Total cycles      : %d\n", steps == -1 ? total : steps);
    printf("  Total meals eaten : %d\n\n", total);
    printf("  Meals per philosopher:\n");

    /* print each philosopher's meal count with bar and starvation flag */
    int starvation = 0;
    for (int i = 0; i < NUM_PHILS; i++) {

        /* build meal bar */
        char bar[64] = "";
        for (int b = 0; b < meals[i] && b < 20; b++)
            strcat(bar, "█");

        /* starvation check */
        if (meals[i] == 0 || (avg > 0 && meals[i] < avg / 2)) {
            printf(DARK_RED "  P%d  %-12s :  %2d  %s  <- STARVATION WARNING\n" RESET,
                   i, phil_names[i], meals[i], bar);
            starvation = 1;
        } else {
            printf(GREEN "  P%d  %-12s :  %2d  %s\n" RESET,
                   i, phil_names[i], meals[i], bar);
        }
    }

    /* overall fairness */
    printf("\n");
    if (total == 0)
        printf(DARK_RED "  Fairness : DEADLOCK — no philosopher ate\n" RESET);
    else if (starvation)
        printf(MAGENTA  "  Fairness : WARNING — potential starvation detected\n" RESET);
    else
        printf(GREEN    "  Fairness : OK — no starvation detected\n" RESET);

    printf(BOLD CYAN "  ────────────────────────────────────────────────────────\n\n" RESET);
}
