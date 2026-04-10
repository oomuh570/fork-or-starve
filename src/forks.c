#include "philosopher.h"

int left_fork(int i){
  return i;
}

int right_fork(int i){
  return (i + 1) % NUM_PHILS;
}
