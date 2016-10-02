#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "success.h"
#include "setting.h"
#include "bufferManager.h"
#include "idle.h"
#include "probability.h"
#include "limits.h"
#include "perModel.h"

extern double gElapsedTime;
extern simSpec gSpec;
extern std11 gStd;

int timeFrameLength(int byteLength, double dataRate){
	int timeLength;

	timeLength = gStd.phyHeader + 4 * ((gStd.macService + 8* (gStd.macHeader + byteLength + gStd.macFcs) + gStd.macTail + (4 * dataRate - 1)) / (4 * dataRate));
	//printf("%f, %d\n", dataRate, timeLength);

	//printf("%f\n", dataRate);
	if(timeLength<0){
		printf("Time length < 0\n");
		exit(38);
	}
	return timeLength;
}

void transmission(staInfo sta[], apInfo *ap){
	/*
	APは非飽和の場合は考慮していない
	フレームの長さや伝送速度の違いは考慮していない
	*/
	bool fUpColl = false;
	bool fNoUplink = false;
	bool fNoDownlink = false;
	int minBackoff;
	int i;
	//int txFrameLength = 0;
	//int txTimeFrameLength;
	double totalTime = 0;
	int upNode, downNode;
	//int rxSta = INT_MAX;
	int apLength = 0;
	int staLength = 0;
	minBackoff = selectNode(ap, sta, &fUpColl, &fNoUplink, &fNoDownlink, &upNode, &downNode);
	//printf("%d\n", minBackoff);

	gSpec.chance++;

	if(fNoUplink==true && fNoDownlink==true){
		printf("Error! (106)\n");
	}else{
		//calculatePhyRate(ap, sta, &upNode, &downNode);
	}

	if(fUpColl==false){
		//ここでいいかはかなり怪しい
		if(gSpec.proMode==6&&(sta[upNode-1].dataRate==0||ap->dataRate==0)){
			if(upNode-1>=0){
				sta[upNode-1].fTx = false;
			}
			apLength = timeFrameLength(1500, 6);
			goto MODE6;
		}
		//Uplinl successed.
		if(fNoDownlink==false){
			ap->sumFrameLengthInBuffer -= ap->buffer[0].lengthMsdu;
			ap->byteSuccFrame += ap->buffer[0].lengthMsdu;
			//txFrameLength = ap->buffer[0].lengthMsdu;
			apLength = timeFrameLength(ap->buffer[0].lengthMsdu, ap->dataRate);
			/*if(txFrameLength<ap->buffer[0].lengthMsdu){
				txFrameLength = ap->buffer[0].lengthMsdu;
			}*/
			ap->buffer[0].lengthMsdu = 0;
			ap->sumDelay += (gElapsedTime - ap->buffer[0].timeStamp);
			ap->buffer[0].timeStamp = 0;
			ap->numSuccFrame++;
			ap->numTxFrame++;
			//ap->numPrimFrame++;
			swapAp(ap);
		}

		if(fNoUplink==false){
			gSpec.succ++;
			for(i=0; i<gSpec.numSta; i++){
				if(sta[i].fTx==true){
					//sta[i].backoffCount = rand() % (sta[i].cw + 1);
					sta[i].fTx = false;
					sta[i].sumFrameLengthInBuffer -= sta[i].buffer[0].lengthMsdu;
					sta[i].byteSuccFrame += sta[i].buffer[0].lengthMsdu;
					staLength = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
					/*if(txFrameLength<sta[i].buffer[0].lengthMsdu){
						txFrameLength = sta[i].buffer[0].lengthMsdu;
					}*/
					sta[i].buffer[0].lengthMsdu = 0;
					//printf("%f\n", sta[i].buffer[0].timeStamp);
					sta[i].sumDelay += (gElapsedTime - sta[i].buffer[0].timeStamp);
					sta[i].buffer[0].timeStamp = 0;
					sta[i].numTxFrame++;
					sta[i].numSuccFrame++;
					swapSta(&sta[i]);
				}else{
					sta[i].fTx = false;
					/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
						sta[i].backoffCount--;
					}*/
				}
			}
		}

		MODE6:
		//txTimeFrameLength = gStd.phyHeader + 4 * ((gStd.macService + 8* (gStd.macHeader + txFrameLength + gStd.macFcs) + gStd.macTail + (4 * gStd.dataRate - 1)) / (4 * gStd.dataRate));
		if(apLength==0&&staLength==0){
			printf("Frame length error.\n");
		}else if(gSpec.proMode==6 || gSpec.proMode==7){
			if(apLength<=staLength){
				totalTime = staLength + gStd.sifs + gStd.timeAck;
			}else{
				totalTime = apLength + gStd.sifs + gStd.timeAck;
			}
		}else if(apLength<=staLength){
			totalTime = (double)minBackoff * gStd.slot + staLength + gStd.sifs + gStd.timeAck;
		}else{
			totalTime = (double)minBackoff * gStd.slot + apLength + gStd.sifs + gStd.timeAck;
		}
		gElapsedTime += totalTime;
		gSpec.sumTotalTime += totalTime;
		arriveAp(ap, totalTime);
		for(i=0; i<gSpec.numSta; i++){
			arriveSta(&sta[i], totalTime);
			sta[i].fRx = false;
			sta[i].fTx = false;
		}
	}else{
		//Uplink failed.
		gSpec.coll++;
		if(fNoDownlink==false){
			ap->sumFrameLengthInBuffer -= ap->buffer[0].lengthMsdu;
			ap->byteSuccFrame += ap->buffer[0].lengthMsdu;
			//txFrameLength = ap->buffer[0].lengthMsdu;
			apLength = timeFrameLength(ap->buffer[0].lengthMsdu, ap->dataRate);
			/*if(txFrameLength<ap->buffer[0].lengthMsdu){
				txFrameLength = ap->buffer[0].lengthMsdu;
			}*/
			ap->buffer[0].lengthMsdu = 0;
			ap->sumDelay += (gElapsedTime - ap->buffer[0].timeStamp);
			ap->buffer[0].timeStamp = 0;
			ap->numSuccFrame++;
			ap->numTxFrame++;
			//ap->numPrimFrame++;
			swapAp(ap);
		}
		for(i=0; i<gSpec.numSta; i++){
			if(sta[i].fTx==true){
				//sta[i].backoffCount = rand() % (sta[i].cw + 1);
				sta[i].fTx = false;
				sta[i].numTxFrame++;
				sta[i].numCollFrame++;

				if(staLength<timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate)){
					staLength = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
				}
				/*if(txFrameLength<sta[i].buffer[0].lengthMsdu){
					txFrameLength = sta[i].buffer[0].lengthMsdu;
				}*/
			}else{
				sta[i].fTx = false;
				/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
					sta[i].backoffCount--;
					sta[i].fRx = false;
				}*/
			}
		}
		//printf("%d\n", staLength);
		if(apLength==0&&staLength==0){
			printf("Frame length error.\n");
		}else if(gSpec.proMode==6 || gSpec.proMode ==7){
			if(apLength<=staLength){
				totalTime = staLength + gStd.sifs + gStd.timeAck;
			}else{
				totalTime = apLength + gStd.sifs + gStd.timeAck;
			}
		}else if(apLength<=staLength){
			totalTime = (double)minBackoff * gStd.slot + staLength + gStd.sifs + gStd.timeAck;
		}else{
			totalTime = (double)minBackoff * gStd.slot + apLength + gStd.sifs + gStd.timeAck;
		}
		//txTimeFrameLength = gStd.phyHeader + 4 * ((gStd.macService + 8* (gStd.macHeader + txFrameLength + gStd.macFcs) + gStd.macTail + (4 * gStd.dataRate - 1)) / (4 * gStd.dataRate));
		//totalTime = txTimeFrameLength + gStd.sifs + gStd.timeAck;
		gElapsedTime += totalTime;
		gSpec.sumTotalTime += totalTime;
		arriveAp(ap, totalTime);
		for(i=0; i<gSpec.numSta; i++){
			arriveSta(&sta[i], totalTime);
			sta[i].fRx = false;
			sta[i].fTx = false;
		}
	}
}
