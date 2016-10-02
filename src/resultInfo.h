#ifndef _resultInfo_h
#define _resultInfo_h

#include "macro.h"

typedef struct resultInformation{
	double aveStaThroughput;
	double apThroughput;
	double aveThroughput;
	double aveStaProColl;
	double apProColl;
	double aveProColl;
	double aveStaDelay;
	double apDelay;
	double aveDelay;
	double proUp[NUM_STA];
	double proSucc;
	double proColl;
	double aveTotalTime;
	double oppJFI;
	double thrJFI;
	double dlyJFI;
}resultInfo;

#endif
