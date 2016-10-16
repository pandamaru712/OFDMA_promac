#include <stdlib.h>
#include <limits.h>
#include "idle.h"
#include "frameGen.h"

extern double gElapsedTime;
extern std11 gStd;
extern simSpec gSpec;

int upContention(staInfo sta[], bool *fUpColl){
	int minBackoff = INT_MAX;
	int i;
	int numTx = 0;

	for(i=0; i<gSpec.numSta; i++){
		if((minBackoff>sta[i].backoffCount)&&(sta[i].buffer[0].lengthMsdu!=0)){
			minBackoff = sta[i].backoffCount;
		}
	}
	if(minBackoff==INT_MAX){
		printf("All STAs don't have a frame.\n");
	}else{
		for(i=0; i<gSpec.numSta; i++){
			if(minBackoff==sta[i].backoffCount){
				sta[i].fTxOne = true;
				sta[i].fTxSecond = false;
				sta[i].backoffCount = rand() % (sta[i].cw + 1);
				numTx++;
			}else{
				sta[i].backoffCount -= minBackoff;
				sta[i].fTxOne = false;
				sta[i].fTxSecond = false;
			}
		}
	}
	if(numTx==0){
		printf("undefined\n");
	}else if(numTx==1){
		*fUpColl = false;
	}else{
		*fUpColl = true;
	}

	return minBackoff;
}

void idle(staInfo sta[], apInfo *ap, int *numTx, bool *fEmpty){
	int i;
	int minBackoff = 0;
	double t;
	int nodeID;
	bool isSta;

	if(*fEmpty==false){
		for(i=0; i<gSpec.numSta; i++){
			if((minBackoff > sta[i].backoffCount)&&(sta[i].buffer[0].lengthMsdu!=0)){
				minBackoff = sta[i].backoffCount;
			}
		}
		if((minBackoff > ap->backoffCount)&&(ap->buffer[0].lengthMsdu!=0)){
			minBackoff = ap->backoffCount;
		}

		for(i=0; i<gSpec.numSta; i++){
			if(sta[i].buffer[0].lengthMsdu!=0){
				sta[i].backoffCount -= minBackoff;
				if(sta[i].backoffCount==0){
					(*numTx)++;
					sta[i].fTxOne = true;
					sta[i].fTxSecond = false;
				}
			}
		}
		if(ap->buffer[0].lengthMsdu!=0){
			ap->backoffCount -= minBackoff;
			if(ap->backoffCount==0){
				(*numTx)++;
				ap->fTx = true;
			}
		}

		gElapsedTime += (double)(gStd.slot * minBackoff);
	}else{
		nodeID = rand() % (gSpec.numSta + 1);
		if(nodeID==gSpec.numSta){
			//Arrive frame to AP.
			isSta = false;
			t = poisson(isSta);
			ap->buffer[0].lengthMsdu = traffic(isSta);
			ap->buffer[0].timeStamp = gElapsedTime + t;
			ap->sumFrameLengthInBuffer += ap->buffer[0].lengthMsdu;
			ap->fTx = true;
			*numTx = 1;
		}else{
			//Arrive frame to sta[nodeID].
			isSta = true;
			t = poisson(isSta);
			sta[nodeID].buffer[0].lengthMsdu = traffic(isSta);
			sta[nodeID].buffer[0].timeStamp = gElapsedTime + t;
			sta[nodeID].sumFrameLengthInBuffer += sta[nodeID].buffer[0].lengthMsdu;
			sta[nodeID].fTxOne = true;
			sta[nodeID].fTxSecond = false;
			*numTx = 1;
		}
		gElapsedTime += t;
	}

}
