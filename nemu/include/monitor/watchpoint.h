#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char expr[32];
  int new_val;
  int old_val;
  /* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(WP *p);
int set_watchpoint(char *e);
bool delete_watchpoint(int NO);
void list_watchpoint(void);
WP* scan_watchpoint(void);
#endif
