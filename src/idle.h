#ifndef _idle_h
#define _idle_h
#include "nodeInfo.h"
#include "setting.h"

int upContention(staInfo*, bool*);
void idle(staInfo*, apInfo*, int*, bool*);

#endif
