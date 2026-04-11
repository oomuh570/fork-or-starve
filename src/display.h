/*
 * File: display.h
 * Purpose: Header file for display functions.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

/*
 * Function: print_state
 * Purpose: Prints one philosopher's state to the terminal.
 * Params:
 *   i     - philosopher number
 *   state - current state of the philosopher
 * Returns: void
 */
void print_state(long i, int state);

/*
 * Function: clear_screen
 * Purpose: Clears the terminal for live table refresh.
 * Returns: void
 */
void clear_screen();

/*
 * Function: print_table
 * Purpose: Prints live state table of all philosophers.
 *          Shows state, forks held, and meal count.
 * Returns: void
 */
void print_table();

/*
 * Function: display_thread
 * Purpose: Runs as a separate thread, refreshes table every 0.5 seconds.
 * Params:  arg - unused
 * Returns: NULL
 */
void *display_thread(void *arg);

#endif
