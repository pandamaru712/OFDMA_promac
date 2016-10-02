#ifndef _probability_h
#define _probability_h

#include "nodeInfo.h"
#include "setting.h"
#include "nodeInfo.h"

int selectNode(apInfo*, staInfo*, bool*, bool*, bool*, int*, int*);
void calculateProbability(staInfo*, apInfo*);
void initializeMatrix(void);
void solveLP(void);

#endif
