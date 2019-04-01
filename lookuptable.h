#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "defines.h"

#define PRECISION 10000
double* sintable;
double* costable;
STYPE*  diodetable;

void inisintable();
void inicostable();
void iniDiodeTable();
double getSin(double value);
//STYPE getDiode(STYPE i);
double getCos(double value);



