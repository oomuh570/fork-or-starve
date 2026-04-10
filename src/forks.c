/*
 * File: forks.c
 * Purpose: Gives the left and right fork index for a philosopher.
 */

#include "philosopher.h"

/*
 * Function: left_fork
 * Purpose: Returns the left fork number for a philosopher.
 * Params:
 *   i - philosopher number
 * Returns: left fork index
 */
int left_fork(int i){
  return i;
}

/*
 * Function: right_fork
 * Purpose: Returns the right fork number for a philosopher.
 * Params:
 *   i - philosopher number
 * Returns: right fork index
 */
int right_fork(int i){
  return (i + 1) % NUM_PHILS;
}
