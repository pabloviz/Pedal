#include "defines.h"
#include "effects.h"
#define DIODE_N 15
#define DIODE_N2 64
#define nmax 32768
#define step nmax/DIODE_N2
#define mult_f 0.01;

//int lims[DIODE_N]; //= {400,800,1400,2300,3300,4500,4700,4900,5100,5300,6000,6500,8000,20000};
//int lims[] = {400,900,1600,2600,3700,5000,5200,5400,5600,5800,6400,7000,10000,20000};

//float mult[DIODE_N]; //= {1.1,1,0.8,0.7,0.6,0.5,0.45,0.4,0.35,0.3,0.25,0.15,0.1,0.05};

//int last[DIODE_N]; //= {440,940,1500,2200,2860,3510,3600,3680,3750,3810,3960,4050,4350,4850};
STYPE getDiode(STYPE i);
void iniDiode(int th, int soft, int hard);

void iniDiode2(double fin1_p, double multini, double linfin, double softfin, double hardfin);

STYPE getDiode2(STYPE x);
