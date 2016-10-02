#ifndef _setting_h
#define _setting_h

#include <stdio.h>
#include "macro.h"

typedef enum boolean{
	false,
	true
}bool;

typedef struct standard{
	char *std;
	int dataRate;
	int ackRate;
	int rtsRate;
	int ctsRate;
	int ackLength;
	int rtsLength;
	int ctsLength;
	int timeAck;
	int timeRts;
	int timeCts;
	int sifs;
	int difs;
	int eifs;
	int slot;
	int afterColl;
	int afterSucc;
	int ackTimeout;
	int ctsTimeout;
	int retryLimit;
	int cwMin;
	int cwMax;
	int aMsduMax;
	int aMsduInAMpduMax;
	int aMpduMax;
	int phyHeader;   //us
	int macService;   //bits
	int macHeader;   //bytes
	int macFcs;   //bytes
	int macTail;   //bytes
}std11;

typedef struct specification{
	bool fDebug;
	bool fFd;
	bool fOfdma;
	int numSta;
	int simTime;   //s
	int bufferSize;
	int bufferSizeByte;   //kB
	int numTrial;
	int trafficPattern;
	double lambdaAp;   //1/us
	double lambdaSta;
	int delayMode;
	bool fOutput;
	char filename[STR_MAX];
	int areaSize;   //m
	FILE *output;
	double SIC;   //Self-interference cancekation
	double ICIth;   //Inter-client interference threshold
	double noise;
	int chance;
	int succ;
	int coll;
	long sumTotalTime;
	int proMode;
	int position;
	double delayPower;
	double giveU;
	int delaySTA;
	int seed;
	int rateMode;
	double bandWidth;
}simSpec;

void simSetting(int, char**);

#endif
