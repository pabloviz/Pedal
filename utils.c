#include "utils.h"
#include "defines.h"


STYPE sigadd(STYPE a, STYPE b){
	int tmp = (int)a + (int)b;
	if(tmp>MAXVALUE) return MAXVALUE;
	if (tmp<-MAXVALUE) return -MAXVALUE;
	return a + b;
}

void bufftozero(char* buff, int ini, int buff_size){
	STYPE* sbuff = (STYPE*)(&buff[0]);
	for(int i=ini; i<buff_size/BXS;++i) sbuff[i]=0;
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
	for(int i=A; i<=B; i+=2){
		buff[i] = l*(i-vx)*(i-vx) + vy;	
	}
}
void savebuff(char * buff_f, int buff_size_f, char * buff_d, int buff_size_d, int* pos){
	//printf("starting savebuff. pos = %d\n",*pos);
	int p = *pos;
	for (int i=0; i<buff_size_f; i+=1){ //INTERLEAVED
		if(p>=buff_size_d) p=0;
		buff_d[p]= buff_f[i];
		++p;
		//buff_d[p+1] = buff_f[i+1]; //INTERLEAVED
		//p+=2; //INTERLEAVED
	}
	//p+=1024;
	//if(p==buff_size_d)p=0;
	*pos = p;
	//printf("ending savebuff. pos = %d\n",*pos);

}
