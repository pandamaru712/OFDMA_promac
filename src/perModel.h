#ifndef _perModel_h
#define _perModel_h

#include "nodeInfo.h"
#include "setting.h"

void calculateRSSI(apInfo*, staInfo*, double*);
void calculateDistance(apInfo*, staInfo*);
void calculateDelay(apInfo*, staInfo*, double*);
void calculatePhyRate(apInfo*, staInfo*, int*, int*, int*);
#endif
