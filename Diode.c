#include "Diode.h"

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
	mult[0] = 2.5;
	last[0] = lims[0]*mult[0];
	float dec = 0.2;
	int inc = th;
	int incsum = inc/2;
	//linear	
	for(int i=1;i<6;++i){
		lims[i] = lims[i-1]+inc;
		inc += incsum;
		mult[i] = mult[i-1]-dec;
	}
	//soft
	inc = th/2; //o 200...
	dec = 1.0/((float)soft+1.0);
	for(int i=6;i<10;++i){
		lims[i] = lims[i-1]+inc;
		mult[i] = mult[i-1]-dec;
	}
	//hard
	inc = 500;
	dec = 0.2;
	int incmult = 3;
	for(int i=10;i<DIODE_N;++i){
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
}


STYPE getDiode(STYPE i){

	
	for(int j=0;j<DIODE_N;++j){
		if(i<=lims[j]){
			if(j==0) return i*mult[j];
			return i*mult[j] + (last[j-1]-(lims[j-1]*mult[j]));
		}
	}
	return 0;
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
