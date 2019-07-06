#include "utils.h"
#include "defines.h"


STYPE max(STYPE a, STYPE b){
	return a - ( (a-b) & -(a<b));
}
STYPE min(STYPE a, STYPE b){
	return a - ( (a-b) & -(a>b));
}

inline STYPE mult_sat(STYPE val, double mult){
	int tmpval = (int)(val*mult);
	int mask1 = -(tmpval>MAXVALUE);
	int mask2 = -(tmpval<-MAXVALUE);
	return (MAXVALUE&mask1)|(-MAXVALUE&mask2)|(tmpval&(!mask1)&(!mask2));


}

STYPE sigadd(STYPE a, STYPE b){
	int tmp = (int)a + (int)b;
	if(tmp>MAXVALUE) return MAXVALUE;
	if (tmp<-MAXVALUE) return -MAXVALUE;
	return a + b;
}

void buff_volume_adjust(char * buff, int ini, int buff_size,double volume){
	//struct timeval tv1,tv2;
	//gettimeofday(&tv1,NULL);
	short* sbuff = (short*)&(buff[0]);
	if(ini%2==1) printf("ini ha de ser un nombre parell");
	for(int i=0; i<buff_size/BXS; i+=2){
		int new = (int)sbuff[i]*volume;
		if(new>MAXVALUE)new=MAXVALUE;
		else if (new<-MAXVALUE)new=-MAXVALUE;
		else sbuff[i]=(STYPE)new;
	}
	//gettimeofday(&tv2,NULL);
	//printf("time: %d\n",(tv2.tv_usec-tv1.tv_usec));
}

void printbuff(char* buff, int buff_size){

	int fd = open("plot", O_CREAT | O_RDWR, 0644);
	STYPE* sbuff = (STYPE*)(&buff[0]);
	char buffer[20];
	sprintf(buffer,"#X Y\n");
	write(fd,buffer,strlen(buffer));
	for(int i=0; i<buff_size/BXS;i+=2){
		STYPE s = sbuff[i];
		sprintf(buffer,"%d %d\n",i,s);
		write(fd,buffer,strlen(buffer));
	}
	//printf("\n\n\naaaaaaaaaaa\n\n\n");
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
		//buff[i] = 0;
		//printf("%d\n",Y);
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
