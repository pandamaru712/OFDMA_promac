#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "result.h"
#include "macro.h"

extern double gElapsedTime;
extern simSpec gSpec;
extern int gNumOptimization;
extern double gTotalTimeOptimization;
extern double gTimeSimulation;
extern int gNumHalfDuplex;
extern int gNumFullDuplex_J;
extern int gNumFullDuplex_K;
extern int gNumFullDuplex_J_K;
extern int gNumOFDMA;
extern int gNumOFDMAandFullDuplex;

void simulationResult(staInfo sta[], apInfo *ap, resultInfo *result, int trialID){
	int i;

	long rNumFrameTx = 0;
	long rNumFrameSucc = 0;
	long rNumFrameColl = 0;
	long rNumFrameLost = 0;
	long rByteFrameSucc = 0;
	long rNumPrimFrame = 0;
	double rDelay = 0.0;
	double tempColl = 0;
	long opp = 0;
	double dly = 0;
	double thr = 0;
	long totalTXOP = 0;

	for(i=0; i<gSpec.numSta; i++){
		rNumFrameTx += sta[i].numTxFrame;
		rNumFrameSucc += sta[i].numSuccFrame;
		rNumFrameColl += sta[i].numCollFrame;
		rNumFrameLost += sta[i].numLostFrame;
		rNumPrimFrame += sta[i].numPrimFrame;
		rByteFrameSucc += sta[i].byteSuccFrame;
		rDelay += sta[i].sumDelay / sta[i].numSuccFrame;
	}

	//printf("%ld, %ld, %ld\n\n", rNumFrameTx, rNumFrameSucc, rNumFrameColl);

	/*if(rNumFrameSucc!=rNumPrimFrame){
		printf("Somthing is wrong.\n", rNumFrameTx, rNumFrameSucc);
	}*/
	if(ap->numTxFrame!=(ap->numSuccFrame+ap->numCollFrame)){
		printf("Somthing is wrong.%f\n", gElapsedTime);
	}

	//printf("%f, %f\n", rDelay, ap->sumDelay);
	//printf("\n");
	/*for(i=0; i<NUM_STA; i++){
		tempColl += (double)sta[i].numCollFrame / sta[i].numTxFrame;
		printf("%ld, ", sta[i].numTxFrame);
		//printf("%f, ", (double)sta[i].numCollFrame / sta[i].numTxFrame);
	}
	printf("\n\n");*/
	/*for(i=0; i<NUM_STA; i++){
		printf("%f\n", sta[i].sumDelay / sta[i].numSuccFrame/1000);//ms
	}
	printf("\n");*/
	result->aveStaThroughput += (double)rByteFrameSucc * 8 / gElapsedTime / gSpec.numSta;
	result->apThroughput += (double)ap->byteSuccFrame * 8 / gElapsedTime;
	result->aveThroughput += (double)(rByteFrameSucc + ap->byteSuccFrame) * 8 /gElapsedTime;
	//printf("%f", tempColl/gSpec.numSta);
	result->aveStaProColl += tempColl / gSpec.numSta;//(double)rNumFrameColl / rNumFrameSucc;
	//result->apProColl += (double)ap->numCollFrame / ap->numPrimFrame;
	//result->aveProColl += (double)(rNumFrameColl + ap->numCollFrame) / (rNumPrimFrame + ap->numPrimFrame);

	result->aveStaDelay += rDelay / NUM_STA;//rNumFrameSucc;
	result->apDelay += ap->sumDelay / ap->numSuccFrame;
	result->aveDelay += (rDelay + ap->sumDelay) / (rNumFrameSucc + ap->numSuccFrame);

	result->proColl += (double)gSpec.coll / gSpec.chance;
	result->aveTotalTime += (double)gSpec.sumTotalTime / gSpec.chance;

	result->totalNumOptimization += gNumOptimization;
	result->totalTimeOptimization += gTotalTimeOptimization;
	result->aveTimeOptimization += gTotalTimeOptimization / gNumOptimization;
	result->totalTimeSimulation += gTimeSimulation;

	totalTXOP = gNumHalfDuplex + gNumFullDuplex_J + gNumFullDuplex_K + gNumFullDuplex_J_K + gNumOFDMA + gNumOFDMAandFullDuplex;
	result->proHalfDuplex += (double)gNumHalfDuplex / totalTXOP;
	result->proFullDuplex_J += (double)gNumFullDuplex_J / totalTXOP;
	result->proFullDuplex_K += (double)gNumFullDuplex_K / totalTXOP;
	result->proFullDuplex_J_K += (double)gNumFullDuplex_J_K / totalTXOP;
	result->proOFDMA += (double)gNumOFDMA / totalTXOP;
	result->proOFDMAandFullduplex += (double)gNumOFDMAandFullDuplex / totalTXOP;

	for(i=0; i<NUM_STA; i++){
		opp += pow(sta[i].numTxFrame, 2);
		thr += pow(sta[i].byteSuccFrame * 8 / gElapsedTime, 2);
		dly += pow(sta[i].sumDelay / sta[i].numSuccFrame, 2);
	}
	result->oppJFI += (double)pow(rNumFrameTx, 2) / (NUM_STA * opp);
	result->thrJFI += pow((double)rByteFrameSucc * 8 / gElapsedTime, 2) / (NUM_STA * thr);
	result->dlyJFI += pow(rDelay, 2) / (NUM_STA * dly);

	for(i=0; i<NUM_STA; i++){
		//printf("%ld\n", sta[i].numSuccFrame);
		//result->proUp[i] += (double)sta[i].numSuccFrame / rNumFrameSucc;
		//printf("%ld\n", sta[i].numTxFrame);
	}

	if(trialID==(gSpec.numTrial-1)){
		printf("\n***** Result *****\n");
		printf("STA1台あたりのスループットは%f Mbit/s\n", result->aveStaThroughput / gSpec.numTrial);
		printf("APのスループットは%f Mbit/s\n", result->apThroughput / gSpec.numTrial);
		printf("システムスループットは%f Mbit/s\n", result->aveThroughput / gSpec.numTrial);
		printf("STAの平均衝突率は%f \n", result->aveStaProColl / gSpec.numTrial);
		printf("%f, %d, %d\n", result->proColl / gSpec.numTrial, gSpec.coll, gSpec.chance);
		printf("%f\n", result->aveTotalTime / gSpec.numTrial);
		//printf("APの衝突率は%f \n", result->apProColl / gSpec.numTrial);
		//printf("システムの平均衝突率は%f \n", result->aveProColl / gSpec.numTrial);
		printf("STAの平均遅延は%f us\n", result->aveStaDelay / gSpec.numTrial);
		printf("APの遅延は%f us\n", result->apDelay / gSpec.numTrial);
		printf("システムの平均遅延は%f us\n", result->aveDelay / gSpec.numTrial);
		/*for(i=0; i<NUM_STA; i++){
			printf("p_u[%d] = %f\n", i, sta[i].sumDelay / sta[i].numSuccFrame);
		}*/
		printf("送信機会のFairness indexは%f \n", result->oppJFI / gSpec.numTrial);
		printf("待機時間のFairness indexは%f \n", result->dlyJFI / gSpec.numTrial);
		printf("スループットのFairness indexは%f \n", result->thrJFI / gSpec.numTrial);
		printf("総最適化回数は%d，総最適化時間は%f秒，平均所要時間は%f秒\n", result->totalNumOptimization, result->totalTimeOptimization, result->aveTimeOptimization/gSpec.numTrial);
		printf("試行回数は%d，総シミュレーション時間は%f秒，平均%f秒\n", gSpec.numTrial, result->totalTimeSimulation, result->totalTimeSimulation/gSpec.numTrial);
		printf("Half-duplex: %f\n", result->proHalfDuplex/gSpec.numTrial);
		printf("Full-duplex J: %f\n", result->proFullDuplex_J/gSpec.numTrial);
		printf("Full-duplex K: %f\n", result->proFullDuplex_K/gSpec.numTrial);
		printf("Full-duplex J K: %f\n", result->proFullDuplex_J_K/gSpec.numTrial);
		printf("OFDMA: %f\n", result->proOFDMA/gSpec.numTrial);
		printf("OFDMA and Full-duplex: %f\n", result->proOFDMAandFullduplex/gSpec.numTrial);
		printf("**********\n");
		if(gSpec.fOutput==true){
			fprintf(gSpec.output, "\n***** Result *****\n");
			fprintf(gSpec.output, "STA1台あたりのスループットは%f Mbit/s\n", result->aveStaThroughput / gSpec.numTrial);
			fprintf(gSpec.output, "APのスループットは%f Mbit/s\n", result->apThroughput / gSpec.numTrial);
			fprintf(gSpec.output, "システムスループットは%f Mbit/s\n", result->aveThroughput / gSpec.numTrial);
			fprintf(gSpec.output, "STAの平均衝突率は%f \n", result->aveStaProColl / gSpec.numTrial);
			fprintf(gSpec.output, "APの衝突率は%f \n", result->apProColl / gSpec.numTrial);
			fprintf(gSpec.output, "システムの平均衝突率は%f \n", result->aveProColl / gSpec.numTrial);
			fprintf(gSpec.output, "STAの平均遅延は%f us\n", result->aveStaDelay / gSpec.numTrial);
			fprintf(gSpec.output, "APの遅延は%f us\n", result->apDelay / gSpec.numTrial);
			fprintf(gSpec.output, "システムの平均遅延は%f us\n", result->aveDelay / gSpec.numTrial);
			fprintf(gSpec.output, "総最適化回数は%d，総最適化時間は%f秒，平均所要時間は%f秒\n", result->totalNumOptimization, result->totalTimeOptimization, result->aveTimeOptimization/gSpec.numTrial);
			fprintf(gSpec.output, "試行回数は%d，総シミュレーション時間は%f秒，平均%f秒\n", gSpec.numTrial, result->totalTimeSimulation, result->totalTimeSimulation/gSpec.numTrial);
			fprintf(gSpec.output, "Half-duplex: %f\n", result->proHalfDuplex/gSpec.numTrial);
			fprintf(gSpec.output, "Full-duplex J: %f\n", result->proFullDuplex_J/gSpec.numTrial);
			fprintf(gSpec.output, "Full-duplex K: %f\n", result->proFullDuplex_K/gSpec.numTrial);
			fprintf(gSpec.output, "Full-duplex J K: %f\n", result->proFullDuplex_J_K/gSpec.numTrial);
			fprintf(gSpec.output, "OFDMA: %f\n", result->proOFDMA/gSpec.numTrial);
			fprintf(gSpec.output, "OFDMA and Full-duplex: %f\n", result->proOFDMAandFullduplex/gSpec.numTrial);
			fprintf(gSpec.output, "**********\n\n\n");
		}
	}
}
