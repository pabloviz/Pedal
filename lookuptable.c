#include "lookuptable.h"
#include <stdlib.h>

#include <stdio.h>


void ascitointfile(){
	int fd = open("diode.table", O_RDONLY);
	int wr = open("diode.table.num", O_RDWR | O_CREAT);
	if (fd<0) {printf("error opening diode table\n"); exit(1);}
	char buff;
	STYPE num = 0;
	int index = 0;
	while ( (read(fd,&buff,1)) > 0){	
		if (buff=='\n') {
			//printf("%d\n",num);
			//char numero*[sizeof(STYPE)];
			//numero[0] = 
			write(wr,&num,2);
			
			num = 0;	
		}
		else if(buff>='0' && buff<='9') num = num*10 + buff-'0';
	}
	
}

void iniDiodeTable(){
	//ascitointfile();
	int fd = open("diode.table.num", O_RDONLY);
	if (fd<0) {printf("error opening diode table num\n"); exit(1);}
	diodetable = (STYPE*)malloc(32768);
	int res = read(fd,diodetable,32768*sizeof(STYPE));
	//for(int i=0; i<32768; ++i) printf("diodetable[%d] = %d\n",i,diodetable[i]);
	//exit(1); 
}


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

/*STYPE getDiode(STYPE i){
	if (i<0) i = -i;
	return diodetable[i];
}*/


double getSin(double value){
	value = (value/(2.0*PI) - (int)(value/(2.0*PI)));
	if (value<0) value = 1-value;
	return sintable[(int)(((double)PRECISION*(double)value))];
}
double getCos(double value){
	value = (value/(2.0*PI) - (int)(value/(2.0*PI)));
	return costable[(int)(((double)PRECISION*(double)value))];
}

