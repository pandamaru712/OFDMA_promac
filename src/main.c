#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include "nodeInfo.h"
#include "setting.h"
#include "initialization.h"
#include "success.h"
#include "result.h"
#include "macro.h"
#include "probability.h"
#include "success.h"
#include "perModel.h"

#include "engine.h"
#include "matrix.h"
#include "tmwtypes.h"

int gNumOptimization;
double gTotalTimeOptimization;
double gTimeSimulation;
double gElapsedTime;
std11 gStd;
simSpec gSpec;
FILE *gFileSta;
FILE *gFileTopology;

Engine *gEp;
double r[(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)] = {};//{-1, -4, -45, -51, -29, -42, -16, -1, -25, -39, -24, -35, -3, -23, -1, -56, -78, -10, -11, -34, -22, -1, -7, -67, -45, -23, -65, -55, -1, -76, -12, -6, -95, -67, -52, -1};
double pro[NUM_STA+1][NUM_STA+1][NUM_STA+1];
double dummyA[NUM_STA*2][(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
double A[NUM_STA*2][(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
double u[NUM_STA*2];
double dummyAeq[2][(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
double Aeq[2][(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
double beq[2] = {100, 0};
double lb[(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)] = {};
double ub[(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)] = {};
double distance[NUM_STA+1][NUM_STA+1] = {};

void showProgression(int*);

/*void defineArrays(void){
	r = (double *)malloc(sizeof(double)*powint(NUM_STA+1, 3));
	initializeDoubleArray(r, powint(NUM_STA+1, 3), 0);
	u = (double *)malloc(sizeof(double)*NUM_STA*2);
	initializeDoubleArray(u, NUM_STA*2, 0);
	lb = (double *)malloc(sizeof(double)*powint(NUM_STA+1, 3));
	initializeDoubleArray(lb, powint(NUM_STA+1, 3), 0);
	ub = (double *)malloc(sizeof(double)*powint(NUM_STA+1, 3));
	initializeDoubleArray(ub, powint(NUM_STA+1, 3), 0);
}

void freeArrays(void){
	free(r);
	free(u);
	free(lb);
	free(ub);
}*/

int main(int argc, char *argv[]){
	struct timeval start, end;

	//Check option values from command line.
	//checkOption(argc, argv);
	//Apply option values to simulation settings.
	printf("Start simulation.\n");

	simSetting(argc,argv);
	printf("Set simulation parameters.\n");
	/*if((gFileSta=fopen("sta's buffer.txt", "w"))==NULL){
		printf("File cannot open! 3");
		exit(33);
	}*/

	staInfo *sta;
	sta = (staInfo *)malloc(sizeof(staInfo)*gSpec.numSta);
	//defineArrays();

	//sta = (staInfo*)malloc(sizeof(staInfo)*gSpec.numSta);
	apInfo ap;
	resultInfo result;
	//Intialize result information.
	initializeResult(&result);

	int numTx = 0;
	int trialID;
	bool fEmpty = false;
	double lastBeacon = 0;
	int previousCount = 0;

	if(gSpec.position==3){
		gFileTopology = fopen("topology_9.txt", "r");
		if(gFileTopology==NULL){
			printf("Can not open topology file.\n");
			exit(92);
		}
	}

	if(!(gEp = engOpen(""))){
		fprintf(stderr, "\nCan't start MATLAB engine.\n");
		return EXIT_FAILURE;
	}
	printf("Open MATLAB engine.\n");

	for (trialID=0; trialID<gSpec.numTrial; trialID++){
		gettimeofday(&start, NULL);
		printf("\n***** %d/%d *****\n", trialID+1, gSpec.numTrial);
		srand(trialID);
		numTx = 0;
		fEmpty = false;
		lastBeacon = 0;
		initializeNodeInfo(sta, &ap);
		calculateDistance(&ap, sta);
		gElapsedTime = gStd.difs;
		gNumOptimization = 0;
		gTotalTimeOptimization = 0;
		gTimeSimulation = 0;
		initializeMatrix();
		printf("Initialization NodeInfo and Matrix.\n");
		if(gSpec.proMode!=6 && gSpec.proMode!=7){
			calculateProbability(sta, &ap);
		}
		previousCount = 0;
		#ifdef PROGRESS
			printf("                     :   0%%");
		#endif
		for( ;gElapsedTime<gSpec.simTime*1000000; ){
			transmission(sta, &ap);

			if(lastBeacon+100000<gElapsedTime){
				if(gSpec.proMode!=0 && gSpec.proMode!=3 && gSpec.proMode!=5 && gSpec.proMode!=6 && gSpec.proMode!=7){
					calculateProbability(sta, &ap);
				}
				lastBeacon = gElapsedTime;
			}
			processPrintf("%f\n", gElapsedTime);
			#ifdef PROGRESS
				showProgression(&previousCount);
			#endif
		}
		gettimeofday(&end, NULL);
		gTimeSimulation = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000;
		simulationResult(sta, &ap, &result, trialID);
	}

	if(gSpec.fOutput==true){
		fclose(gSpec.output);
	}
	//fclose(gFileSta);
	free(sta);
	//freeArrays();
	printf("Free memory space.\n");

	engEvalString(gEp, "close;");
	engClose(gEp);
	if(gSpec.position==3){
		fclose(gFileTopology);
	}
	printf("Close MATLAB.\nFinish.\n");
	return 0;
}

void showProgression(int *previousCount){
	double progression;
	int count;
	int i;
	char str[256] = {};
	char str2[256] = {};
	if(gElapsedTime<gSpec.simTime*1000000){
		progression = gElapsedTime / (gSpec.simTime * 1000000);
	}else{
		progression = 1;
	}
	count = (int)(progression * 100) / 5;
	//printf("%d, %d | ", *previousCount, count);
	fflush(stdout);
	if(*previousCount<count){
		for(i=0; i<count; i++){
			strcat(str, "#");
		}
		for(; i<20; i++){
			strcat(str, " ");
		}
		if(count<2){
			sprintf(str2, " :   %d%%", count*5);
			strcat(str, str2);
		}else if(count<20){
			sprintf(str2, " :  %d%%", count*5);
			strcat(str, str2);
		}else{
			sprintf(str2, " : 100%%\n");
			strcat(str, str2);
		}
		printf("\r%s", str);
	}
	*previousCount = count;
}
