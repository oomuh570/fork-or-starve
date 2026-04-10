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

#endif
