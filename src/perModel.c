#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "nodeInfo.h"
#include "perModel.h"
#include "limits.h"

extern double r[(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
extern double u[NUM_STA*2];
extern double ub[(NUM_STA+1)*(NUM_STA+1)*(NUM_STA+1)];
extern double gElapsedTime;
extern std11 gStd;
extern double distance[NUM_STA+1][NUM_STA+1];

double sellectPhyRate(double);

void calculateDistance(apInfo *ap, staInfo sta[]){
	int down, up;
	//down, upはダミー含めてののsta番号
	for(down=0; down<NUM_STA+1; down++){
		for(up=0; up<NUM_STA+1; up++){
			if(down!=0 && up==0){
				distance[down][up] = sqrt(pow(sta[down-1].x-ap->x, 2) + pow(sta[down-1].y-ap->y, 2));
			}else if(down==0 && up!=0){
				distance[down][up] = sqrt(pow(sta[up-1].x-ap->x, 2) + pow(sta[up-1].y-ap->y, 2));
			}else if(down==up){
				distance[down][up] = 0;
				//printf("Error 107\n");
			}else{
				distance[down][up] = sqrt(pow(sta[down-1].x-sta[up-1].x, 2) + pow(sta[down-1].y-sta[up-1].y, 2));
			}
		}
	}
}

double mw2dbm(double mw){
	double dbm;
	dbm = 10*log10(mw);
	return dbm;
}

double dbm2mw(double dbm){
	double mw;
	mw = pow(10, dbm/10);
	return mw;
}

double shannon(double sinr){
	double capacity;
	capacity = gSpec.bandWidth * log2(1+sinr);
	return capacity;
}

void calculateRSSI(apInfo *ap, staInfo sta[], double delay[]){
	double rssi = 1;
	double rssi_j = 1;
	double rssi_k = 1;
	//double distance = sqrt(pow(sta->x, 2) + pow(sta->y, 2));
	int i, j, k;
	double uplink, downlink;
	//double noise = -91.63;
	double ICI;   //Inter-client interference
	double ICI_j, ICI_k;
	double snr;
	double snr_j, snr_k;
	double sinr;
	double sinr_j, sinr_k;
	double r_mat[NUM_STA+1][NUM_STA+1][NUM_STA+1] = {};
	double txPower;

	processPrintf("calculateRSSI starts\n");
	/*
	if(isTxSta==false){
		RSSI = ap->txPower + ap->antennaGain + sta->antennaGain - (30*log10(distance) + 47);
	}else{
		RSSI = sta->txPower + ap->antennaGain + sta->antennaGain - (30*log10(distance) + 47);
	}*/
	for(i=0; i<NUM_STA+1; i++){
		for(j=0; j<NUM_STA+1; j++){
			for(k=0; k<NUM_STA+1; k++){//
			if((i!=0&&i==j)||(i!=0&&i==k)){
				//printf("");
				//printf("%d, %d, %d\n", i, j, k);
				r_mat[i][j][k] = 0;
			}else if(i+j+k==0){
				r_mat[i][j][k] = 0;
			}else if(j==0 && k==0 && i!=0){   //下りのみ
				rssi = ap->txPower + ap->antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][0]) + 47);
				snr = rssi - gSpec.noise;// pow(10,(rssi)/10)/(pow(10,(gSpec.noise)/10)+pow(10,(gSpec.ICI)/10));
				if(snr<9.63){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					r_mat[i][j][k] = shannon(dbm2mw(snr));//downlink = 20*log2(1+snr);
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					r_mat[i][j][k] = shannon(dbm2mw(snr)) * pow(delay[j], gSpec.delayPower);
				}
				//printf("%f\n", ap->dataRate);
			}else if(i==0 && j!=0 && k==0){
				rssi_j = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
				snr_j = rssi_j - gSpec.noise;
				if(snr_j<9.63){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					r_mat[i][j][k] = shannon(dbm2mw(snr_j));
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					r_mat[i][j][k] = shannon(dbm2mw(snr_j)) * pow(delay[j], gSpec.delayPower);
					//printf("%f = %f * %f / 10000\n", r_mat[i][j], shannon(dbm2mw(snr)), delay[j]);
				}
			}else if(i==0 && j==0 && k!=0){
				rssi_k = sta[k-1].txPower + sta[k-1].antennaGain + ap->antennaGain - (30*log10(distance[0][k]) + 47);
				snr_k = rssi_k - gSpec.noise;
				if(snr_k<9.63){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					r_mat[i][j][k] = shannon(dbm2mw(snr_k));
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					r_mat[i][j][k] = shannon(dbm2mw(snr_k)) * pow(delay[k], gSpec.delayPower);
					//printf("%f = %f * %f / 10000\n", r_mat[i][j], shannon(dbm2mw(snr)), delay[j]);
				}
			}else if(j!=0 && k!=0 && i==0){  //上りのみ
				if(j==k){
					rssi_j = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
					snr_j = rssi_j - gSpec.noise;
					if(snr_j<9.63){
						r_mat[i][j][k] = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						r_mat[i][j][k] = shannon(dbm2mw(snr_j));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						r_mat[i][j][k] = shannon(dbm2mw(snr_j)) * pow(delay[j], gSpec.delayPower);
						//printf("%f = %f * %f / 10000\n", r_mat[i][j], shannon(dbm2mw(snr)), delay[j]);
					}
				}else{
					rssi_j = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
					rssi_k = sta[k-1].txPower + sta[k-1].antennaGain + ap->antennaGain - (30*log10(distance[0][k]) + 47);
					snr_j = rssi_j - gSpec.noise;
					snr_k = rssi_k - gSpec.noise;
					if(snr_j<9.63){
						r_mat[i][j][k] = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						r_mat[i][j][k] = shannon(dbm2mw(snr_j)) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						r_mat[i][j][k] = shannon(dbm2mw(snr_j)) * pow(delay[j], gSpec.delayPower) / NUM_MULTIPLEX;
						//printf("%f = %f * %f / 10000\n", r_mat[i][j], shannon(dbm2mw(snr)), delay[j]);
					}
					if(snr_k<9.63){
						r_mat[i][j][k] += 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						r_mat[i][j][k] += shannon(dbm2mw(snr_k)) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						r_mat[i][j][k] += shannon(dbm2mw(snr_k)) * pow(delay[k], gSpec.delayPower) / NUM_MULTIPLEX;
						//printf("%f = %f * %f / 10000\n", r_mat[i][j], shannon(dbm2mw(snr)), delay[j]);
					}
					if(snr_j<9.63||snr_k<9.63){
						r_mat[i][j][k] = 0;
					}
					//printf("%f\n", sta[*upNode-1].dataRate);
				}
			}else if(i!=0 && j!=0 && k==0){
				rssi = ap->txPower + ap->antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][0]) + 47);
				snr = rssi - gSpec.noise - gSpec.ICIth;
				if(snr<9.63){
					downlink = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					downlink = shannon(dbm2mw(snr));
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					downlink = shannon(dbm2mw(snr));// * delay[0] / 10000;
				}
				ICI = sta[j-1].txPower + sta[j-1].antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][j]) + 47);
				sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
				//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
				if(sinr>=snr){
					rssi = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
					sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));// * delay[j] / 10000;
					}
				}else{
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[j-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][j]) + 47;
					if(txPower>=sta[j-1].txPower){
						printf("ICI Error\n");
					}
					rssi = txPower + sta[j-1].antennaGain + ap->antennaGain - 30*log10(distance[0][j]) - 47;
					sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));// * delay[j] / 10000;
					}
					//printf("%f\n", txPower);
				}
				if(downlink==0||uplink==0){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					r_mat[i][j][k] = (downlink + uplink) * pow(delay[j], gSpec.delayPower);
				}else{
					r_mat[i][j][k] = downlink + uplink;
				}

				if(i==j){
					r_mat[i][j][k] = 0;
				}
				//printf("%f\n", ap->dataRate);
				//printf("%f\n", sta[*upNode-1].dataRate);
			}else if(i!=0 && j==0 && k!=0){
				rssi = ap->txPower + ap->antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][0]) + 47);
				snr = rssi - gSpec.noise - gSpec.ICIth;
				if(snr<9.63){
					downlink = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					downlink = shannon(dbm2mw(snr));
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					downlink = shannon(dbm2mw(snr));// * delay[0] / 10000;
				}
				ICI = sta[k-1].txPower + sta[k-1].antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][k]) + 47);
				sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
				//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
				if(sinr>=snr){
					rssi = sta[k-1].txPower + sta[k-1].antennaGain + ap->antennaGain - (30*log10(distance[0][k]) + 47);
					sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));// * delay[j] / 10000;
					}
				}else{
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[k-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][k]) + 47;
					if(txPower>=sta[k-1].txPower){
						printf("ICI Error\n");
					}
					rssi = txPower + sta[k-1].antennaGain + ap->antennaGain - 30*log10(distance[0][k]) - 47;
					sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));// * delay[j] / 10000;
					}
					//printf("%f\n", txPower);
				}
				if(downlink==0||uplink==0){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					r_mat[i][j][k] = (downlink + uplink) * pow(delay[k], gSpec.delayPower);
				}else{
					r_mat[i][j][k] = downlink + uplink;
				}

				if(i==k){
					r_mat[i][j][k] = 0;
				}
				//printf("%f\n", ap->dataRate);
				//printf("%f\n", sta[*upNode-1].dataRate);
			}else if(i!=0&&j!=0&&j==k){
				rssi = ap->txPower + ap->antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][0]) + 47);
				snr = rssi - gSpec.noise - gSpec.ICIth;
				if(snr<9.63){
					downlink = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					downlink = shannon(dbm2mw(snr));
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					downlink = shannon(dbm2mw(snr));// * delay[0] / 10000;
				}
				ICI = sta[j-1].txPower + sta[j-1].antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][j]) + 47);
				sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
				//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
				if(sinr>=snr){
					rssi = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
					sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));// * delay[j] / 10000;
					}
				}else{
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[j-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][j]) + 47;
					if(txPower>=sta[j-1].txPower){
						printf("ICI Error\n");
					}
					rssi = txPower + sta[j-1].antennaGain + ap->antennaGain - 30*log10(distance[0][j]) - 47;
					sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));// * delay[j] / 10000;
					}
					//printf("%f\n", txPower);
				}
				if(downlink==0||uplink==0){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					r_mat[i][j][k] = (downlink + uplink) * pow(delay[j], gSpec.delayPower);
				}else{
					r_mat[i][j][k] = downlink + uplink;
				}
			}else{   //3台
				rssi = ap->txPower + ap->antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][0]) + 47);
				snr = rssi - gSpec.noise - gSpec.ICIth;
				if(snr<9.63){
					downlink = 0;
				}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
					downlink = shannon(dbm2mw(snr));
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					downlink = shannon(dbm2mw(snr));// * delay[0] / 10000;
				}
				ICI_j = sta[j-1].txPower + sta[j-1].antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][j]) + 47);
				ICI_k = sta[k-1].txPower + sta[k-1].antennaGain + sta[i-1].antennaGain - (30*log10(distance[i][k]) + 47);
				sinr_j = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI_j)));
				sinr_k = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI_k)));
				if(sinr_j > sinr_k){
					sinr = sinr_k;
				}else{
					sinr = sinr_j;
				}
				//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
				if(sinr>=snr){
					rssi_j = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
					rssi_k = sta[k-1].txPower + sta[k-1].antennaGain + ap->antennaGain - (30*log10(distance[0][k]) + 47);
					sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr_j<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					if(sinr_k<9.63){
						uplink += 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink += shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink += shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					if(sinr_j<9.63||sinr_k<9.63){
						uplink = 0;
					}
				}else if(sinr_j>=snr && sinr_k<snr){
					rssi_j = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0][j]) + 47);
					sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr_j<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[k-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][k]) + 47;
					if(txPower>=sta[k-1].txPower){
						printf("ICI Error\n");
					}
					rssi_k = txPower + sta[k-1].antennaGain + ap->antennaGain - 30*log10(distance[0][k]) - 47;
					sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr_k<9.63){
						uplink += 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink += shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink += shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					if(sinr_j<9.63||sinr_k<9.63){
						uplink = 0;
					}
				}else if(sinr_j<snr && sinr_k>=snr){
					rssi_k = sta[k-1].txPower + sta[k-1].antennaGain + ap->antennaGain - (30*log10(distance[0][k]) + 47);
					sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr_k<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[j-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][j]) + 47;
					if(txPower>=sta[j-1].txPower){
						printf("ICI Error\n");
					}
					rssi_j = txPower + sta[j-1].antennaGain + ap->antennaGain - 30*log10(distance[0][j]) - 47;
					sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr_j<9.63){
						uplink += 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink += shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink += shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					if(sinr_j<9.63||sinr_k<9.63){
						uplink = 0;
					}
				}else{
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[j-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][j]) + 47;
					if(txPower>=sta[j-1].txPower){
						printf("ICI Error\n");
					}
					rssi_j = txPower + sta[j-1].antennaGain + ap->antennaGain - 30*log10(distance[0][j]) - 47;
					sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink = 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink = shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink = shannon(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
					//printf("%f\n", ICI);
					txPower = ICI - sta[k-1].antennaGain - sta[i-1].antennaGain + 30*log10(distance[i][k]) + 47;
					if(txPower>=sta[k-1].txPower){
						printf("ICI Error\n");
					}
					rssi_k = txPower + sta[k-1].antennaGain + ap->antennaGain - 30*log10(distance[0][k]) - 47;
					sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
					if(sinr<9.63){
						uplink += 0;
					}else if(gSpec.proMode==0||gSpec.proMode==2||gSpec.proMode==3||gSpec.proMode==5||gSpec.proMode==6){
						uplink += shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;
					}else if(gSpec.proMode==1||gSpec.proMode==4){
						uplink += shannon(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))) / NUM_MULTIPLEX;// * delay[j] / 10000;
					}
					if(sinr_j<9.63||sinr_k<9.63){
						uplink = 0;
					}
					//printf("%f\n", txPower);
				}
				if(downlink==0||uplink==0){
					r_mat[i][j][k] = 0;
				}else if(gSpec.proMode==1||gSpec.proMode==4){
					//未定義
					r_mat[i][j][k] = (downlink + uplink) * pow(delay[k], gSpec.delayPower);
				}else{
					r_mat[i][j][k] = downlink + uplink;
				}
			}
		}

			/*if(i==j){
				r_mat[i][j] = 0;
			}else if(i==0){
				RSSI = sta[j-1].txPower + sta[j-1].antennaGain + ap->antennaGain - (30*log10(distance[0, j)) + 47);
				uplink = 20*log2(1+pow(10,(RSSI-noise)/10));
				r_mat[i][j] = uplink;
			}else if(j==0){
				RSSI = ap->txPower + ap->antennaGain + sta[i-1].antennaGain - (30*log10(distance(ap, sta, i, 0)) + 47);
				downlink = 20*log2(1+pow(10,(RSSI-noise)/10));
				r_mat[i][j] = downlink;
			}else if(i!=j){
				RSSI = sta[j-1].txPower + ap->antennaGain + sta[j-1].antennaGain - (30*log10(distance(ap, sta, 0, j)) + 47);
				snr = pow(10,RSSI/10) / (pow(10,noise/10) + pow(10,(sta->txPower-gSpec.SIC)/10));
				uplink = 20*log2(1+snr);
				RSSI = ap->txPower + ap->antennaGain + sta[i].antennaGain - (30*log10(distance(ap, sta, i, 0)) + 47);
				ICInterference = sta[j-1].txPower + sta[j-1].antennaGain + sta[i-1].antennaGain - (30*log10(distance(ap, sta, i, j)) + 47);
				snr = pow(10,(RSSI)/10)/(pow(10,(noise)/10)+pow(10,(ICInterference)/10));
				downlink = 20*log2(1+snr);
				r_mat[i][j] = uplink+downlink;
			}else{
				printf("RSSI error!\n");
			}*/
		}
	}

	processPrintf("rate calculation ends\n");
	ratePrintf("\n***** Rate Matrix *****\n");
	for(k=0;k<NUM_STA+1;k++){
		for(i=0;i<NUM_STA+1;i++){
			for(j=0;j<NUM_STA+1;j++){
				if((i!=0&&i==j)||(i!=0&&1==k)){
					if(r_mat[i][j][k]>0){
						printf("rate error (%d, %d, %d)\n", i, j, k);
					}
				}
				if(r_mat[i][j][k]>0){
					//printf("(%d, %d, %d)\n", i, j, k);
					r[(NUM_STA+1)*(NUM_STA+1)*k+i*(NUM_STA+1)+j] = -r_mat[i][j][k];
					ratePrintf("%f, ", r[(NUM_STA+1)*(NUM_STA+1)*k+i*(NUM_STA+1)+j]);
					ub[(NUM_STA+1)*(NUM_STA+1)*k+i*(NUM_STA+1)+j] = 100;
				}else{
					r[(NUM_STA+1)*(NUM_STA+1)*k+i*(NUM_STA+1)+j] = -r_mat[i][j][k];
					ratePrintf("%f, ", r[(NUM_STA+1)*(NUM_STA+1)*k+i*(NUM_STA+1)+j]);
					ub[(NUM_STA+1)*(NUM_STA+1)*k+i*(NUM_STA+1)+j] = 0;
				}
			}
			ratePrintf("\n");
		}
		ratePrintf("\n");
	}
	ratePrintf("***** Rate Matrix *****\n\n");

	processPrintf("calculateRSSI ends");
}

void calculateDelay(apInfo *ap, staInfo sta[], double delay[]){
	//double delay[NUM_STA+1] = {};
	int i;
	double temp=0;
	int num=0;
	double minDelay = gElapsedTime;

	if(gSpec.proMode==1||gSpec.proMode==2||gSpec.proMode==4){
		for(i=1; i<=NUM_STA; i++){
			if(sta[i-1].buffer[0].lengthMsdu!=0){
				delay[i] = gElapsedTime - sta[i-1].buffer[0].timeStamp;
				temp+= delay[i];
				num++;
				if(minDelay>delay[i]){
					minDelay = delay[i];
				}
			}
		}
		if(num<NUM_STA){
			if(num>0){
				for(i=1; i<=NUM_STA; i++){
					if(sta[i-1].buffer[0].lengthMsdu==0){
						delay[i] = minDelay;
						temp+= delay[i];
					}
				}
			}else{
				for(i=1; i<=NUM_STA; i++){
					delay[i] = gStd.difs;
					temp+=delay[i];
				}
			}
		}
	}
	delay[0] = temp/NUM_STA;//gStd.difs;
	/*for(i=0; i<=NUM_STA; i++){
		printf("%f, ", delay[i]);
		printf("\n");
	}*/
	temp += gStd.difs;

	if(gSpec.proMode==2){
		for(i=0; i<NUM_STA; i++){
			u[i] = delay[i+1] / temp / 2;
		}
		for(i=NUM_STA; i<NUM_STA*2; i++){
			u[i] = delay[i-NUM_STA] / temp / 2;
		}
	}
}

void calculatePhyRate(apInfo *ap, staInfo sta[], int *upNodeOne, int *upNodeSecond, int *downNode){
	double rssi;   //dBm
	double snr;   //dBm
	double sinr;   //dBm
	double txPower;   //dBm
	double ICI;   //dBm
	double rssi_j, rssi_k;
	double sinr_j, sinr_k;
	double ICI_j, ICI_k;

	//printf("calculatePhyRate %d %d\n", *downNode, *upNode);

	if(*upNodeOne==0&& *upNodeSecond==0 && *downNode!=0){
		//printf("down half\n");
		rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
		snr = rssi - gSpec.noise;// pow(10,(rssi)/10)/(pow(10,(gSpec.noise)/10)+pow(10,(gSpec.ICI)/10));
		ap->dataRate = sellectPhyRate(snr);//shannon(dbm2mw(snr));//downlink = 20*log2(1+snr);
		//printf("%f\n", ap->dataRate);
	}else if(*upNodeOne!=0 && *upNodeSecond==0 && *downNode==0){
		//printf("up half\n");
		rssi = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
		snr = rssi - gSpec.noise;
		sta[*upNodeOne-1].dataRate = sellectPhyRate(snr)/ NUM_MULTIPLEX;//shannon(dbm2mw(snr));
		//printf("%f\n", sta[*upNode-1].dataRate);
	}else if(*upNodeOne==0 && *upNodeSecond!=0 && *downNode==0){
		//printf("up half\n");
		rssi = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeSecond]) + 47);
		snr = rssi - gSpec.noise;
		sta[*upNodeSecond-1].dataRate = sellectPhyRate(snr)/ NUM_MULTIPLEX;//shannon(dbm2mw(snr));
		//printf("%f\n", sta[*upNode-1].dataRate);
	}else if(*upNodeOne!=0 && *upNodeSecond!=0 && *downNode==0){
		if(*upNodeOne==*upNodeSecond){
			rssi = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
			snr = rssi - gSpec.noise;
			sta[*upNodeOne-1].dataRate = sellectPhyRate(snr);
		}else{
			rssi = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
			snr = rssi - gSpec.noise;
			sta[*upNodeOne-1].dataRate = sellectPhyRate(snr)/ NUM_MULTIPLEX;
			rssi = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeSecond]) + 47);
			snr = rssi - gSpec.noise;
			sta[*upNodeSecond-1].dataRate = sellectPhyRate(snr)/ NUM_MULTIPLEX;
		}
	}else if(*upNodeOne==0 && *downNode==0 && *upNodeSecond==0){
		printf("Error 876\n");
	}else if(*upNodeOne!=0 && *upNodeSecond==0 && *downNode!=0){
		if(*upNodeOne==*downNode){
			printf("Up and down node are the same.\n");
		}
		//printf("full duplex\n");
		rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
		snr = rssi - gSpec.noise;
		ICI = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][*upNodeOne]) + 47);
		sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
		//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
		if(sinr>=snr-gSpec.ICIth){
			ap->dataRate = sellectPhyRate(sinr);//shannon(dbm2mw(sinr));
			rssi = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
			sta[*upNodeOne-1].dataRate = sellectPhyRate(mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))));
			/*if(sta[*upNode].dataRate<6){
				sta[*upNode].dataRate = 6;
			}*/
		}else{
			ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
			//printf("%f\n", ICI);
			txPower = ICI - sta[*upNodeOne-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeOne]) + 47;
			if(txPower>=sta[*upNodeOne-1].txPower){
				printf("ICI Error\n");
			}
			rssi = txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeOne]) - 47;
			sinr = dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC));
			if(sinr<9.63){
				//printf("1:%d, %d, %d\n", *downNode, *upNodeOne, *upNodeSecond);
				//printf("%f\n", distance[0][*upNodeOne]);
				//printf("rssi=%f, sinr=%f\n", rssi, sinr);
			}
			sta[*upNodeOne-1].dataRate = sellectPhyRate(mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))));
			/*if(sta[*upNode].dataRate<6){
				sta[*upNode].dataRate = 6;
			}*/
			rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
			sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
			ap->dataRate = sellectPhyRate(sinr);//shannon(dbm2mw(sinr));
		}
	}else if(*upNodeOne==0 && *upNodeSecond!=0 && *downNode!=0){
		if(*upNodeSecond==*downNode){
			printf("Up and down node are the same.\n");
		}
		//printf("full duplex\n");
		rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
		snr = rssi - gSpec.noise;
		ICI = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][*upNodeSecond]) + 47);
		sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
		//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
		if(sinr>=snr-gSpec.ICIth){
			ap->dataRate = sellectPhyRate(sinr);//shannon(dbm2mw(sinr));
			rssi = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeSecond]) + 47);
			sta[*upNodeSecond-1].dataRate = sellectPhyRate(mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))));
			/*if(sta[*upNode].dataRate<6){
				sta[*upNode].dataRate = 6;
			}*/
		}else{
			ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
			//printf("%f\n", ICI);
			txPower = ICI - sta[*upNodeSecond-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeSecond]) + 47;
			if(txPower>=sta[*upNodeSecond-1].txPower){
				printf("ICI Error\n");
			}
			rssi = txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeSecond]) - 47;
			sinr = dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC));
			if(sinr<9.63){
				//printf("2: %d, %d, %d\n", *downNode, *upNodeOne, *upNodeSecond);
				//printf("%f\n", distance[0][*upNodeOne]);
				//printf("rssi=%f, sinr=%f\n", rssi, sinr);
			}
			sta[*upNodeSecond-1].dataRate = sellectPhyRate(mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))));
			/*if(sta[*upNode].dataRate<6){
				sta[*upNode].dataRate = 6;
			}*/
			rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
			sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
			ap->dataRate = sellectPhyRate(sinr);//shannon(dbm2mw(sinr));
		}
	}else if(*downNode!=0&&*upNodeOne!=0&&*upNodeOne==*upNodeSecond){
		//printf("full duplex\n");
		rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
		snr = rssi - gSpec.noise;
		ICI = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][*upNodeOne]) + 47);
		sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
		//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
		if(sinr>=snr-gSpec.ICIth){
			ap->dataRate = sellectPhyRate(sinr);//shannon(dbm2mw(sinr));
			rssi = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
			sta[*upNodeOne-1].dataRate = sellectPhyRate(mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))));
			/*if(sta[*upNode].dataRate<6){
				sta[*upNode].dataRate = 6;
			}*/
		}else{
			ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
			//printf("%f\n", ICI);
			txPower = ICI - sta[*upNodeOne-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeOne]) + 47;
			if(txPower>=sta[*upNodeOne-1].txPower){
				printf("ICI Error\n");
			}
			rssi = txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeOne]) - 47;
			sinr = dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC));
			if(sinr<9.63){
				//printf("3: %d, %d, %d\n", *downNode, *upNodeOne, *upNodeSecond);
				//printf("%f\n", distance[0][*upNodeOne]);
				//printf("rssi=%f, sinr=%f\n", rssi, sinr);
			}
			sta[*upNodeOne-1].dataRate = sellectPhyRate(mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC))));
			/*if(sta[*upNode].dataRate<6){
				sta[*upNode].dataRate = 6;
			}*/
			rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
			sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI)));
			ap->dataRate = sellectPhyRate(sinr);//shannon(dbm2mw(sinr));
		}
	}else{
		if(*upNodeOne==*downNode){
			printf("Up and down node are the same.\n");
		}
		if(*upNodeSecond==*downNode){
			printf("Up and down node are the same.\n");
		}
		rssi = ap->txPower + ap->antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][0]) + 47);
		snr = rssi - gSpec.noise - gSpec.ICIth;

		ICI_j = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][*upNodeOne]) + 47);
		ICI_k = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + sta[*downNode-1].antennaGain - (30*log10(distance[*downNode][*upNodeSecond]) + 47);
		sinr_j = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI_j)));
		sinr_k = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI_k)));
		if(sinr_j > sinr_k){
			sinr = sinr_k;
		}else{
			sinr = sinr_j;
		}
		//printf("%f, %f, %f, %f\n", rssi, ICI, snr, sinr);
		if(sinr>=snr){
			if(sinr_j>sinr_k){
				sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI_k)));
				ap->dataRate = sellectPhyRate(sinr);
			}else{
				sinr = mw2dbm(dbm2mw(rssi)/(dbm2mw(gSpec.noise)+dbm2mw(ICI_j)));
				ap->dataRate = sellectPhyRate(sinr);
			}
			rssi_j = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
			rssi_k = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeSecond]) + 47);
			sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeOne-1].dataRate = sellectPhyRate(sinr_j) / NUM_MULTIPLEX;
			sta[*upNodeSecond].dataRate = sellectPhyRate(sinr_k) / NUM_MULTIPLEX;
		}else if(sinr_j>=snr && sinr_k<snr){
			rssi_j = sta[*upNodeOne-1].txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeOne]) + 47);
			sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeOne-1].dataRate = sellectPhyRate(sinr_j) / NUM_MULTIPLEX;

			ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
			//printf("%f\n", ICI);
			txPower = ICI - sta[*upNodeSecond-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeSecond]) + 47;
			if(txPower>=sta[*upNodeSecond-1].txPower){
				printf("ICI Error\n");
			}
			rssi_k = txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeSecond]) - 47;
			sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeSecond-1].dataRate = sellectPhyRate(sinr_k) / NUM_MULTIPLEX;
			ap->dataRate = sellectPhyRate(snr);
		}else if(sinr_j<snr && sinr_k>=snr){
			rssi_k = sta[*upNodeSecond-1].txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - (30*log10(distance[0][*upNodeSecond]) + 47);
			sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeSecond-1].dataRate = sellectPhyRate(sinr_k) / NUM_MULTIPLEX;

			ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
			//printf("%f\n", ICI);
			txPower = ICI - sta[*upNodeOne-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeOne]) + 47;
			if(txPower>=sta[*upNodeOne-1].txPower){
				printf("ICI Error\n");
			}
			rssi_j = txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeOne]) - 47;
			sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeOne-1].dataRate = sellectPhyRate(sinr_j) / NUM_MULTIPLEX;
			ap->dataRate = sellectPhyRate(snr);
		}else{
			ICI =mw2dbm((dbm2mw(gSpec.ICIth)-1)*dbm2mw(gSpec.noise));
			//printf("%f\n", ICI);
			txPower = ICI - sta[*upNodeOne-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeOne]) + 47;
			if(txPower>=sta[*upNodeOne-1].txPower){
				printf("ICI Error\n");
			}
			rssi_j = txPower + sta[*upNodeOne-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeOne]) - 47;
			sinr_j = mw2dbm(dbm2mw(rssi_j)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeOne-1].dataRate = sellectPhyRate(sinr_j) / NUM_MULTIPLEX;

			txPower = ICI - sta[*upNodeSecond-1].antennaGain - sta[*downNode-1].antennaGain + 30*log10(distance[*downNode][*upNodeSecond]) + 47;
			if(txPower>=sta[*upNodeSecond-1].txPower){
				printf("ICI Error\n");
			}
			rssi_k = txPower + sta[*upNodeSecond-1].antennaGain + ap->antennaGain - 30*log10(distance[0][*upNodeSecond]) - 47;
			sinr_k = mw2dbm(dbm2mw(rssi_k)/(dbm2mw(gSpec.noise)+dbm2mw(ap->txPower-gSpec.SIC)));
			sta[*upNodeSecond].dataRate = sellectPhyRate(sinr_k) / NUM_MULTIPLEX;
			ap->dataRate = sellectPhyRate(snr);
		}
	}
	if(*downNode!=0){
		//printf("AP's data rate: %f\n", ap->dataRate);
	}
	if(*upNodeOne!=0){
		//printf("sta %d's data rate: %f\n", *upNodeOne-1, sta[*upNodeOne-1].dataRate);
	}
	if(*upNodeSecond!=0){
		//printf("sta %d's data rate: %f\n", *upNodeSecond-1, sta[*upNodeSecond-1].dataRate);
	}

}

double sellectPhyRate(double snr){
	double phyRate;

	if(gSpec.rateMode==0){
		phyRate = shannon(dbm2mw(snr));
	}else{
		if(snr<9.63){
			phyRate = 6;
			printf("Phy rate is 0 Mbit/s\n");
			//exit(17);
		}else if(snr<10.63){
			phyRate = 6;
		}else if(snr<12.63){
			phyRate = 9;
		}else if(snr<14.63){
			phyRate = 12;
		}else if(snr<17.63){
			phyRate = 18;
		}else if(snr<21.63){
			phyRate = 24;
		}else if(snr<25.63){
			phyRate = 36;
		}else if(snr<26.63){
			phyRate = 48;
		}else{
			phyRate = 54;
		}
	}

	return phyRate;
}
