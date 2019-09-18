#include "Diode.h"
//Diode N = 15
int lims[DIODE_N]; //int lims[] = {400,900,1600,2600,3700,5000,5200,5400,5600,5800,6400,7000,10000,20000};

float mult[DIODE_N]; //= {1.1,1,0.8,0.7,0.6,0.5,0.45,0.4,0.35,0.3,0.25,0.15,0.1,0.05};

int last[DIODE_N]; /*int lims[] = {400,800,1400,2300,3300,4500,4700,4900,5100,5300,6000,6500,8000,20000};
//int lims[] = {400,900,1600,2600,3700,5000,5200,5400,5600,5800,6400,7000,10000,20000};

float mult[] = {1.1,1,0.8,0.7,0.6,0.5,0.45,0.4,0.35,0.3,0.25,0.15,0.1,0.05};

int last[] = {440,940,1500,2200,2860,3510,3600,3680,3750,3810,3960,4050,4350,4850};
*/
	//th[0-450-100] soft[0-4-10] hard [0-7-10]
void iniDiode(int th, int soft, int hard){
	lims[0] = 400;
	mult[0] = 1.2;
	last[0] = lims[0]*mult[0];
	float dec = 0.1;
	int inc = th;
	int incsum = inc/2;
	//linear
	int i;	
	for(i=1;i<6;++i){
		lims[i] = lims[i-1]+inc;
		inc += incsum;
		mult[i] = mult[i-1]-dec;
	}
	//soft
	inc = th/2; //o 200...
	dec = 0.5/((float)soft+1.0);
	dec = 0.1;
	for(i=6;i<2+soft;++i){
		lims[i] = lims[i-1]+inc;
		mult[i] = mult[i-1]-dec;
	}
	printf("%d %d\n", lims[6],lims[11]);
	//hard
	inc = 500;
	dec = 0.2;
	int incmult = 3;
	int rd = i;
	for(;i<DIODE_N;++i){
		lims[i] = lims[i-1]+inc;
		mult[i] = mult[i-1]-dec;
		inc *= incmult;
		dec = dec - dec/(hard+1);
		if (mult[i] < 0) mult[i] = mult_f;	
	}
	//prev
	for(int i=1;i<DIODE_N;++i){
		last[i] = lims[i]*mult[i] + (last[i-1]-(lims[i-1]*mult[i]));
	}
//	printf("hola\n");
	/*	
	FILE * fp = fopen("diode.csv", "w+");
	for(int k=0; k<32768; ++k){
		fprintf(fp,"%d %d\n",k,getDiode(k));
	}
	rename("diode.csv", "diodefin.csv");
	fclose(fp);

	FILE * fp1 = fopen("example1.csv", "w+");
	FILE * fp2 = fopen("example2.csv", "w+");
	for(int k=0; k<512; ++k){
		int val = sin((double)k/20.0)*7500.0;
		val += cos((double)k/30.0)*2500.0;
		fprintf(fp2,"%d %d\n",k,val);
		int new;
		if (val>0) new = val*0.2 + getDiode(val)*0.8;
		if (val<0) new = val*0.2 - getDiode(-val)*0.8;
		//if (val>0) fprintf(fp1,"%d %d\n",k,getDiode(val)*0.8 + val*0.2);
		STYPE rnd = rand()%512;
		rnd -= 256;
		new += rnd;
		//else fprintf(fp1,"%d %d\n",k,-getDiode(-val)*0.8);
		fprintf(fp1,"%d %d\n",k,new);
	}
	rename("example1.csv", "examplefin1.csv");
	rename("example2.csv", "examplefin2.csv");
	*/

}
STYPE points_x[DIODE_N2];
STYPE points_y[DIODE_N2];
double slopes[DIODE_N2];

void iniDiode2(double fin1_p, double multini, double linfin, double softfin, double hardfin){
	double fin2_p = fin1_p + fin1_p/2;
	STYPE fin1 = fin1_p * DIODE_N2;
	STYPE fin2 = fin2_p * DIODE_N2;
	printf("%d %d\n", fin1, fin2);
	double mult = multini;
	points_x[0] = 0;
	points_y[0] = 0;
	slopes[0] = mult;
	double next_y = mult*step;
	int index = 1;
	double lindec = (multini-linfin)/(fin1-2.0);
	double softdec = (linfin-softfin)/(fin2-fin1);
	double harddec = (softfin-hardfin)/(DIODE_N2-fin2+1.0);
	int i;
	for(i=step; i<=step*(fin1-1);i+=step){
		points_x[index] = i-1;
		points_y[index] = next_y;
		slopes[index] = mult;
		mult -= lindec;
		next_y = slopes[index]*(points_x[index]-points_x[index-1])+points_y[index];
		++index;
	}
	mult = linfin;

	for(i=step*fin1; i<=step*(fin2-1);i+=step){
		points_x[index] = i-1;
		points_y[index] = next_y;
		mult -= softdec;
		slopes[index] = mult;
		next_y = slopes[index]*(points_x[index]-points_x[index-1])+points_y[index];
		++index;
	}
	
	for(i=step*fin2; i<=step*DIODE_N2;i+=step){
		points_x[index] = i-1;
		points_y[index] = next_y;
		mult -= harddec;
		slopes[index] = mult;
		next_y = slopes[index]*(points_x[index]-points_x[index-1])+points_y[index];
		++index;
	}
	for(i=0; i<DIODE_N2;++i) printf("%d %d %hf\n",points_x[i],points_y[i],slopes[i]);	
	FILE * fp = fopen("diode.csv", "w+");
	for(int k=0; k<32768; ++k){
		fprintf(fp,"%d %d\n",k,getDiode2(k));
	}
	rename("diode.csv", "diodefin.csv");
	fclose(fp);
}
STYPE getDiode2(STYPE x){
	int myindex;
	int mystep = step;
	STYPE ret;
	if (x==0) ret=x;
	else if (x>0){
		myindex = x/mystep;
		if (myindex>=DIODE_N2)
			myindex	= DIODE_N2 - 1;
		ret=slopes[myindex]*(x-points_x[myindex])+points_y[myindex];	
	}else{
		myindex = (int)(-x)/mystep;
		if (myindex>=DIODE_N2)
			myindex	= DIODE_N2 - 1;
		ret=-(slopes[myindex]*(-x-points_x[myindex])+points_y[myindex]);	
	}
	//printf("in: %d, out: %d, index=%d, step %d\n",x,ret,myindex,mystep);
	return ret;
}



STYPE getDiode(STYPE i){	
	for(int j=0;j<DIODE_N;++j){
		if(i<=lims[j]){
			if(j==0) return i*mult[j];
			return i*mult[j] + (last[j-1]-(lims[j-1]*mult[j]));
		}
	}
	return i*mult[DIODE_N-1] + (last[DIODE_N-2]-(lims[DIODE_N-2]*mult[DIODE_N-1]));;
	/*
	if(i<0) i = -i;

	if (i<lims[0]) return i*mult[0];
	if (i<lims[1]) return i*mult[1] + (last[0]-(lims[0]*mult[1]));
	if (i<lims[2]) return i*mult[2] + (last[1]-(lims[1]*mult[2]));
	if (i<lims[3]) return i*mult[3] + (last[2]-(lims[2]*mult[3]));
	if (i<lims[4]) return i*mult[4] + (last[3]-(lims[3]*mult[4]));
	if (i<lims[5]) return i*mult[5] + (last[4]-(lims[4]*mult[5]));
	if (i<lims[6]) return i*mult[6] + (last[5]-(lims[5]*mult[6]));
	if (i<lims[7]) return i*mult[7] + (last[6]-(lims[6]*mult[7]));
	if (i<lims[8]) return i*mult[8] + (last[7]-(lims[7]*mult[8]));
	if (i<lims[9]) return i*mult[9] + (last[8]-(lims[8]*mult[9]));
	if (i<lims[10]) return i*mult[10] + (last[9]-(lims[9]*mult[10]));
	if (i<lims[11]) return i*mult[11] + (last[10]-(lims[10]*mult[11]));
	if (i<lims[12]) return i*mult[12] + (last[11]-(lims[11]*mult[12]));
	if (i<lims[13]) return i*mult[13] + (last[12]-(lims[12]*mult[13]));
	else return i*0.01 + (last[13]-(lims[13]*0.01));
	*/

}
