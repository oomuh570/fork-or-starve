/*
 * File: forks.h
 * Purpose: Header file for fork helper functions.
 */

#ifndef FORKS_H
#define FORKS_H

/*
 * Function: left_fork
 * Purpose: Returns the left fork index for a philosopher.
 * Params:
 *   i - philosopher number
 * Returns: left fork index
 */
int left_fork(int i);

/*
 * Function: right_fork
 * Purpose: Returns the right fork index for a philosopher.
 * Params:
 *   i - philosopher number
 * Returns: right fork index
 */
int right_fork(int i);

#endif
