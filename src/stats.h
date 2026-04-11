/*
 * File: stats.h
 * Purpose: Header file for meal tracking and starvation checking.
 */

#ifndef STATS_H
#define STATS_H

/*
 * Function: meal_record
 * Purpose: Adds one meal to a philosopher's total.
 * Params:
 *   i - philosopher number
 * Returns: void
 */
void meal_record(long i);

/*
 * Function: starve_check
 * Purpose: Checks if a philosopher may be starving.
 * Params:
 *   count - current step count
 * Returns: void
 */
void starve_check(int count);

/*
 * Function: print_stats
 * Purpose: Prints final statistics summary when simulation ends.
 *          Shows mode, total cycles, meals per philosopher,
 *          and overall fairness assessment.
 * Returns: void
 */
void print_stats();

#endif