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
	int totalNumOptimization;
	double aveTimeOptimization;
	double totalTimeOptimization;
	double totalTimeSimulation;
	double proHalfDuplex;
	double proFullDuplex_J;
	double proFullDuplex_K;
	double proFullDuplex_J_K;
	double proOFDMA;
	double proOFDMAandFullduplex;
}resultInfo;

#endif
