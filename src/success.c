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

int getMax(int one, int two, int three){
	if(one<two){
		if(two<three){
			return three;
		}else{
			return two;
		}
	}else{
		if(one<three){
			return three;
		}else{
			return one;
		}
	}
}

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
	bool fUpCollOne = false;
	bool fUpCollSecond = false;
	bool fNoUpOne = false;
	bool fNoUpSecond = false;
	bool fNoDownlink = false;
	int minBackoff;
	int i;
	//int txFrameLength = 0;
	//int txTimeFrameLength;
	double totalTime = 0;
	int upNodeOne, upNodeSecond, downNode;
	//int rxSta = INT_MAX;
	int apLength = 0;
	int staLengthOne = 0;
	int staLengthSecond = 0;
	minBackoff = selectNode(ap, sta, &fUpCollOne, &fUpCollSecond, &fNoUpOne, &fNoUpSecond, &fNoDownlink, &upNodeOne, &upNodeSecond, &downNode);
	int maxLength;
	//printf("%d\n", minBackoff);

	gSpec.chance++;

	if(fNoUpOne==true && fNoUpSecond==true && fNoDownlink==true){
		printf("Error! (106)\n");
	}else{
		//calculatePhyRate(ap, sta, &upNode, &downNode);
	}

	if(fUpCollOne==false&&fUpCollSecond==false){
		//ここでいいかはかなり怪しい
		/*if(gSpec.proMode==6&&(sta[upNode-1].dataRate==0||ap->dataRate==0)){
			if(upNode-1>=0){
				sta[upNode-1].fTx = false;
			}
			apLength = timeFrameLength(1500, 6);
			goto MODE6;
		}*/
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

		if(fNoUpOne==false || fNoUpSecond==false){
			gSpec.succ++;
			if(upNodeOne==upNodeSecond){
				for(i=0; i<NUM_STA; i++){
					if(sta[i].fTxOne==true&&sta[i].fTxSecond==true){
						//sta[i].backoffCount = rand() % (sta[i].cw + 1);
						sta[i].fTxOne = false;
						sta[i].fTxSecond = false;
						sta[i].sumFrameLengthInBuffer -= sta[i].buffer[0].lengthMsdu;
						sta[i].byteSuccFrame += sta[i].buffer[0].lengthMsdu;
						staLengthOne = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
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
						sta[i].fTxOne = false;
						sta[i].fTxSecond = false;
						/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
							sta[i].backoffCount--;
						}*/
					}
				}
			}else{
				for(i=0; i<gSpec.numSta; i++){
					if(sta[i].fTxOne==true||sta[i].fTxSecond==true){
						//sta[i].backoffCount = rand() % (sta[i].cw + 1);
						sta[i].fTxOne = false;
						sta[i].sumFrameLengthInBuffer -= sta[i].buffer[0].lengthMsdu;
						sta[i].byteSuccFrame += sta[i].buffer[0].lengthMsdu;
						if(sta[i].fTxOne==true){
							staLengthOne = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
						}else{
							staLengthSecond = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
						}
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
						sta[i].fTxOne = false;
						sta[i].fTxSecond = false;
						/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
							sta[i].backoffCount--;
						}*/
					}
				}
			}
		}

		//MODE6:
		//txTimeFrameLength = gStd.phyHeader + 4 * ((gStd.macService + 8* (gStd.macHeader + txFrameLength + gStd.macFcs) + gStd.macTail + (4 * gStd.dataRate - 1)) / (4 * gStd.dataRate));
		if(apLength==0&&staLengthOne==0&&staLengthSecond==0){
			printf("Frame length error.\n");
		}else if(gSpec.proMode==6 || gSpec.proMode==7){
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = maxLength + gStd.sifs + gStd.timeAck;
		}else{
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = (double)minBackoff * gStd.slot + maxLength + gStd.sifs + gStd.timeAck;
		}
		gElapsedTime += totalTime;
		gSpec.sumTotalTime += totalTime;
		arriveAp(ap, totalTime);
		for(i=0; i<gSpec.numSta; i++){
			arriveSta(&sta[i], totalTime);
			sta[i].fRx = false;
			sta[i].fTxOne = false;
			sta[i].fTxSecond = false;
		}
	}else if(fUpCollOne==true&&fUpCollSecond==false){
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
			if(sta[i].fTxSecond==true){
				//sta[i].backoffCount = rand() % (sta[i].cw + 1);
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				sta[i].sumFrameLengthInBuffer -= sta[i].buffer[0].lengthMsdu;
				sta[i].byteSuccFrame += sta[i].buffer[0].lengthMsdu;
				staLengthSecond = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
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
			}else if(sta[i].fTxOne==true){
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				sta[i].numTxFrame++;
				sta[i].numCollFrame++;
				staLengthOne = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
			}else{
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
					sta[i].backoffCount--;
				}*/
			}
		}
		if(apLength==0&&staLengthOne==0&&staLengthSecond==0){
			printf("Frame length error.\n");
		}else if(gSpec.proMode==6 || gSpec.proMode==7){
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = maxLength + gStd.sifs + gStd.timeAck;
		}else{
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = (double)minBackoff * gStd.slot + maxLength + gStd.sifs + gStd.timeAck;
		}
		gElapsedTime += totalTime;
		gSpec.sumTotalTime += totalTime;
		arriveAp(ap, totalTime);
		for(i=0; i<gSpec.numSta; i++){
			arriveSta(&sta[i], totalTime);
			sta[i].fRx = false;
			sta[i].fTxOne = false;
			sta[i].fTxSecond = false;
		}
	}else if(fUpCollOne==false&&fUpCollSecond==true){
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
			if(sta[i].fTxOne==true){
				//sta[i].backoffCount = rand() % (sta[i].cw + 1);
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				sta[i].sumFrameLengthInBuffer -= sta[i].buffer[0].lengthMsdu;
				sta[i].byteSuccFrame += sta[i].buffer[0].lengthMsdu;
				staLengthOne = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
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
			}else if(sta[i].fTxSecond==true){
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				sta[i].numTxFrame++;
				sta[i].numCollFrame++;
				staLengthSecond = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
			}else{
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
					sta[i].backoffCount--;
				}*/
			}
		}
		if(apLength==0&&staLengthOne==0&&staLengthSecond==0){
			printf("Frame length error.\n");
		}else if(gSpec.proMode==6 || gSpec.proMode==7){
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = maxLength + gStd.sifs + gStd.timeAck;
		}else{
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = (double)minBackoff * gStd.slot + maxLength + gStd.sifs + gStd.timeAck;
		}
		gElapsedTime += totalTime;
		gSpec.sumTotalTime += totalTime;
		arriveAp(ap, totalTime);
		for(i=0; i<gSpec.numSta; i++){
			arriveSta(&sta[i], totalTime);
			sta[i].fRx = false;
			sta[i].fTxOne = false;
			sta[i].fTxSecond = false;
		}
	}else{
		//Both uplink is collided
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
			if(sta[i].fTxOne==true||sta[i].fTxSecond==true){
				//sta[i].backoffCount = rand() % (sta[i].cw + 1);
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				sta[i].numTxFrame++;
				sta[i].numCollFrame++;

				if(sta[i].fTxOne==true){
					staLengthOne = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
				}else{
					staLengthSecond = timeFrameLength(sta[i].buffer[0].lengthMsdu, sta[i].dataRate);
				}
				/*if(txFrameLength<sta[i].buffer[0].lengthMsdu){
					txFrameLength = sta[i].buffer[0].lengthMsdu;
				}*/
			}else{
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
				/*if((sta[i].buffer[0].lengthMsdu!=0)&&(sta[i].backoffCount!=0)){
					sta[i].backoffCount--;
					sta[i].fRx = false;
				}*/
			}
		}
		//printf("%d\n", staLength);
		if(apLength==0&&staLengthOne==0&&staLengthSecond==0){
			printf("Frame length error.\n");
		}else if(gSpec.proMode==6 || gSpec.proMode ==7){
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = maxLength + gStd.sifs + gStd.timeAck;
		}else{
			maxLength = getMax(apLength, staLengthOne, staLengthSecond);
			totalTime = (double)minBackoff * gStd.slot + maxLength + gStd.sifs + gStd.timeAck;
		}
		//txTimeFrameLength = gStd.phyHeader + 4 * ((gStd.macService + 8* (gStd.macHeader + txFrameLength + gStd.macFcs) + gStd.macTail + (4 * gStd.dataRate - 1)) / (4 * gStd.dataRate));
		//totalTime = txTimeFrameLength + gStd.sifs + gStd.timeAck;
		gElapsedTime += totalTime;
		gSpec.sumTotalTime += totalTime;
		arriveAp(ap, totalTime);
		for(i=0; i<gSpec.numSta; i++){
			arriveSta(&sta[i], totalTime);
			sta[i].fRx = false;
			sta[i].fTxOne = false;
			sta[i].fTxSecond = false;
		}
	}
}
