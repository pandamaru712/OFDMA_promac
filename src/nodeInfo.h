#ifndef _nodeInfo_h
#define _nodeInfo_h

#include "setting.h"
#include "macro.h"

extern simSpec gSpec;

typedef struct frameInformation{
	int lengthMsdu;
	double timeStamp;
	//int destination;
}frameInfo;

typedef struct staInformatioin{
	frameInfo buffer[BUFFER_SIZE];
	int backoffCount;
	int cw;
	int retryCount;
	long numTxFrame;
	long numCollFrame;
	long numLostFrame;
	long numSuccFrame;
	long numPrimFrame;
	long byteSuccFrame;
	bool fCollNow;
	int afterColl;
	bool fSuccNow;
	int afterSucc;
	bool fTxOne;
	bool fTxSecond;
	bool fRx;
	long sumFrameLengthInBuffer;
	double sumDelay;
	int waitFrameLength;
	double x;
	double y;
	double txPower;
	double antennaGain;
	double timeNextFrame;
	double distanceAp;
	double dataRate;
	int pos;
}staInfo;

typedef struct apInformation{
	frameInfo buffer[BUFFER_SIZE];
	int backoffCount;
	int cw;
	int retryCount;
	long numTxFrame;
	long numCollFrame;
	long numLostFrame;
	long numSuccFrame;
	long numPrimFrame;
	long byteSuccFrame;
	bool fCollNow;
	int afterColl;
	bool fSuccNow;
	int afterSucc;
	bool fTx;
	long sumFrameLengthInBuffer;
	double sumDelay;
	int waitFrameLength;
	double x;
	double y;
	double txPower;
	double antennaGain;
	double timeNextFrame;
	double dataRate;
}apInfo;

#endif
