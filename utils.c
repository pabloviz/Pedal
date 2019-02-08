#include "utils.h"


int sigadd(int a, int b){

	a-=32767;
	b-=32767;
	//printf("%d, %d\n",a,b);
	int c = a+b;
	c+=32767;
	if (c>100535) c = 65535;
	else if (c<-2) c=0;
	return c;
}

int vecToInt(char upper, char lower){ //TODO: amb shorts m'estalvio feina potser..
	
	int m = 1;
	int value = (upper<<8)+(lower&0x0FF);
	//printf("old: %x %x ",upper,lower);
	//printf("old: %d ",value);
	if(value>32767){//negatiu
		//printf("is negatiu ");
		m=-1;
		value = (~value)&0xFFFF;
		//printf(" ->%d ",value);
		++value;
		//upper = ~upper;
		//lower = ~lower + 1;
	}
	return m*value;
}
