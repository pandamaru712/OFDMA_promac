#include <stdlib.h>
#include <math.h>
#include "initialization.h"
#include "setting.h"
#include "frameGen.h"
#include "macro.h"
#include "limits.h"

extern std11 gStd;
extern FILE *gFileTopology;

void initializeResult(resultInfo *result){
	result->aveStaThroughput = 0.0;
	result->apThroughput = 0.0;
	result->aveThroughput = 0.0;
	result->aveStaProColl = 0.0;
	result->apProColl = 0.0;
	result->aveProColl = 0.0;
	result->aveStaDelay = 0.0;
	result->apDelay = 0.0;
	result->aveDelay = 0.0;
	for(int i=0; i<NUM_STA; i++){
		result->proUp[i] = 0;
	}
	result->proSucc = 0;
	result->proColl = 0;
	result->aveTotalTime = 0;
	result->thrJFI = 0;
	result->oppJFI = 0;
	result->dlyJFI = 0;
	result->aveTimeOptimization = 0;
	result->totalTimeOptimization = 0;
	result->totalNumOptimization = 0;
	result->totalTimeSimulation = 0;
	result->proHalfDuplex = 0;
	result->proFullDuplex_J = 0;
	result->proFullDuplex_K = 0;
	result->proFullDuplex_J_K = 0;
	result->proOFDMA = 0;
	result->proOFDMAandFullduplex = 0;
}

void initializeNodeInfo(staInfo sta[], apInfo* ap){
	int i, j;
	double tempX, tempY;

	for(i=0; i<BUFFER_SIZE; i++){
		ap->buffer[i].lengthMsdu = 0;
		ap->buffer[i].timeStamp = 0.0;
		//ap->buffer[i].destination = INT_MAX;
		/*if(ap->sumFrameLengthInBuffer>(gSpec.bufferSizeByte*1000)){
			ap->waitFrameLength = ap->buffer[i].lengthMsdu;
			ap->sumFrameLengthInBuffer -= ap->buffer[i].lengthMsdu;
			ap->buffer[i].lengthMsdu = 0;
			ap->buffer[i].timeStamp = 0.0;
			break;
		}*/
	}
	ap->buffer[0].lengthMsdu = traffic(false);
	//ap->buffer[0].destination = rand() % NUM_STA;
	ap->sumFrameLengthInBuffer = ap->buffer[0].lengthMsdu;
	ap->waitFrameLength = traffic(false);
	ap->backoffCount = rand() % (gStd.cwMin + 1);
	ap->cw = gStd.cwMin;
	ap->retryCount = 0;
	ap->numTxFrame = 0;
	ap->numCollFrame = 0;
	ap->numLostFrame = 0;
	ap->numSuccFrame = 0;
	ap->numPrimFrame = 0;
	ap->byteSuccFrame = 0;
	ap->fCollNow = false;
	ap->afterColl = 0;
	ap->fSuccNow = false;
	ap->afterSucc = 0;
	ap->fTx = false;
	//ap->sumFrameLengthInBuffer = 0;
	ap->sumDelay = 0.0;
	if(gSpec.position==0 || gSpec.position==1 || gSpec.position==3){
		ap->x = 0.0;
		ap->y = 0.0;
	}else if(gSpec.position==2){
		ap->x = (double)rand() / RAND_MAX * gSpec.areaSize - (double)gSpec.areaSize / 2;
		ap->y = (double)rand() / RAND_MAX * gSpec.areaSize - (double)gSpec.areaSize / 2;
	}
	ap->txPower = 20.0;
	ap->antennaGain = 2.0;
	ap->timeNextFrame = poisson(false);
	ap->dataRate = gStd.dataRate;

	for(i=0; i<gSpec.numSta; i++){
		for(j=0; j<BUFFER_SIZE; j++){
			sta[i].buffer[j].lengthMsdu = 0;
			sta[i].buffer[j].timeStamp = 0.0;
			/*if(sta[i].sumFrameLengthInBuffer>(gSpec.bufferSizeByte*1000)){
				sta[i].waitFrameLength = sta[i].buffer[i].lengthMsdu;
				sta[i].sumFrameLengthInBuffer -= sta[i].buffer[i].lengthMsdu;
				sta[i].buffer[i].lengthMsdu = 0;
				sta[i].buffer[i].timeStamp = 0.0;
				break;
			}*/
		}
		sta[i].buffer[0].lengthMsdu = traffic(true);
		sta[i].sumFrameLengthInBuffer = sta[i].buffer[0].lengthMsdu;
		sta[i].waitFrameLength = traffic(true);
		sta[i].backoffCount = rand() % (gStd.cwMin + 1);
		sta[i].cw = gStd.cwMin;
		sta[i].retryCount = 0;
		sta[i].numTxFrame = 0;
		sta[i].numCollFrame = 0;
		sta[i].numLostFrame = 0;
		sta[i].numSuccFrame = 0;
		sta[i].numPrimFrame = 0;
		sta[i].byteSuccFrame = 0;
		sta[i].fCollNow = false;
		sta[i].afterColl = 0;
		sta[i].fSuccNow = false;
		sta[i].afterSucc = 0;
		sta[i].fTxOne = false;
		sta[i].fTxSecond = false;
		//sta[i].sumFrameLengthInBuffer = 0;
		sta[i].sumDelay = 0.0;
		if(gSpec.position==0){
			sta[i].x = (i / 10 + 1) * 10 * cos((i % 10) * 36 * 3.14 / 180);
			sta[i].y = (i / 10 + 1) * 10 * sin((i % 10) * 36 * 3.14 / 180);
		}else if(gSpec.position==1 || gSpec.position==2){
			sta[i].x = (double)rand() / RAND_MAX * gSpec.areaSize - (double)gSpec.areaSize / 2;
			sta[i].y = (double)rand() / RAND_MAX * gSpec.areaSize - (double)gSpec.areaSize / 2;
		}else{
			fscanf(gFileTopology, "%lf", &tempX);
			fscanf(gFileTopology, "%lf", &tempY);
			sta[i].x = tempX;
			sta[i].y = tempY;
		}
		if(sta[i].x>0){
			if(sta[i].y>0){
				sta[i].pos = 1;
			}else{
				sta[i].pos = 4;
			}
		}else{
			if(sta[i].y>0){
				sta[i].pos = 2;
			}else{
				sta[i].pos = 3;
			}
		}
		positionPrintf("%f, %f, pos %d\n", sta[i].x, sta[i].y, sta[i].pos);
		sta[i].distanceAp = sqrt(pow(sta[i].x, 2)+pow(sta[i].y, 2));
		sta[i].txPower = 20.0;   //dBm
		sta[i].antennaGain = 2.0;   //dBi
		sta[i].timeNextFrame = poisson(true);
		sta[i].dataRate = gStd.dataRate;
	}

	gSpec.chance = 0;
	gSpec.succ = 0;
	gSpec.coll = 0;
	gSpec.sumTotalTime = 0;
}

void initializeDoubleArray(double array[], int sizeArray, double value){
	for(int i=0; i<sizeArray; i++){
		array[i] = value;
	}
}
