#include "utils.h"
#include "defines.h"


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

//interleaved
void quadunion(STYPE * buff, int buff_size, int A, int B, int Y, int power){
	if(B>=buff_size/BXS){
		printf("error in quadunion\n");
		return;
	}
	power = power*(B-A)/4; 
	int vx = A + ((B-A)/2);
	int vy = Y - power;
	double l = ((double)(Y-vy))/((B-vx)*(B-vx));
	//printf("A: %d, B: %d, Y: %d, power: %d, vx: %d, Vy: %d, l: %hf\n",A,B,Y,power,vx,vy, l);
	//if(vy>Y) l= -l;
	for(int i=A; i<=B; i+=2){
		buff[i] = l*(i-vx)*(i-vx) + vy;	
	}

	//quadunion(sbuff,buff_size,pos,i,-limit,-100);
	//}
	/*
	a = -(((i-posn)/2)+posn);
	b = -limit-100;
	l = (100.0)/((i+a)*(i+a));
	for(int j=posn;j<=i;++j){
		if(tipo==1)sbuff[j]=l*(j+a)*(j+a) + b;
	}*/
}
