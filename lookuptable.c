#include "lookuptable.h"
#include <stdlib.h>




void inisintable(){
	sintable = (double*)malloc(PRECISION*sizeof(double));
	for (int i=0; i<PRECISION; ++i){
		sintable[i] = sin((double)i*2.0*PI/(double)PRECISION);
	}
}
void inicostable(){

	costable = (double*)malloc(PRECISION*sizeof(double));
	for (int i=0; i<PRECISION; ++i){
		costable[i] = cos((double)i*2.0*PI/(double)PRECISION);
	}
}

double getSin(double value){
	value = (value/(2.0*PI) - (int)(value/(2.0*PI)));
	if (value<0) value = 1-value;
	return sintable[(int)(((double)PRECISION*(double)value))];
}
double getCos(double value){
	value = (value/(2.0*PI) - (int)(value/(2.0*PI)));
	return costable[(int)(((double)PRECISION*(double)value))];
}

