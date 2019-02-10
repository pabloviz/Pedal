#include "effects.h"
#include "utils.h"
//new
//

int howmany=-1;
char* echobuff = (char*)-1;
void echo(int period, char* buff, int buff_size,int ret, int layers){ //layers default = 1
		if(layers<=0) return;
		if(echobuff == (char*)-1 || howmany==-1){ //TODO cuidado malloc massa gran
			howmany = (ret*1000)/period;
			printf("howmany : %d\n",howmany);
			echobuff_size = buff_size*howmany;
			printf("echobuff_size: %d\n",echobuff_size);
			echobuff = (char*)malloc(echobuff_size);
			echobuff_index = 0;
			for(int i=0; i<echobuff_size; ++i) echobuff[i]=0;
		}
		//double power = 1.0/(double)layers+1;
		int tmp_howmany = howmany/layers;
		int index;
		char temp[buff_size];
		STYPE* sbuff = (STYPE*)&(buff[0]);
		STYPE* sebuff =(STYPE*)&(echobuff[0]);
		//double power_m=-BXS/(layers*buff_size); //en realitat el bxs esta dividinti
		//double power;
		for(int j=1; j<=layers; ++j)
		{
			double power = (double)layers/((layers+1)*j*4.0);
			//double power = 1.0/((layers+1)*j*1.0);
			//if(j==layers) power = 0.05;
			int samples_back = tmp_howmany*j*buff_size;//quantes mostres enrere (8bits)
			if (samples_back%2==1) printf("ei\n");
			if (samples_back>howmany*buff_size){
				printf("error\n");
				return;
			}
			//samples_back/=BXS;//mostres enrere en 16 bits
			//printf("shorts back %d\n",samples_back);
			for(int i=0; i<buff_size/BXS; i+=1){
				if(j==1){
					temp[BXS*i]=buff[BXS*i];
					temp[BXS*i+1]=buff[BXS*i+1];
				}
				int actualindex = echobuff_index*buff_size +BXS*i;//a nivell de byte
				int ecoindex = actualindex - samples_back;
				//printf("index %d\n",index);
				if (ecoindex<0)ecoindex = echobuff_size+ecoindex;
				STYPE new_value = sbuff[i];
				//power = power_m*(l*buff_size/BXS + i)+1.0;
				STYPE old_value = sebuff[ecoindex/BXS]*power;
				//printf("i: %d, index:%d\n",echobuff_index*buff_size/BXS + i,index/BXS);
				//TODO overflow control, maybe in sigadd function
				sbuff[i] = new_value+old_value;
				//new_value = sigadd(old_value,new_value);
				//new_value = old_value + new_value;
				//if(new_value>65535) new_value= new_value|0xFF00;
				//buff[i] = (new_value>>8);
				//buff[i+1] = (new_value&0x0FF);
			}

		}
		//buff_volume_adjust(buff,0,buff_size,0.5);
		//actualitzar echo buffer
		for(int i=0;i<buff_size;++i) {
			echobuff[echobuff_index*buff_size +i] = temp[i]/* * (temp[i]>0)*/;
		}
		++echobuff_index;
		if(echobuff_index >= howmany)echobuff_index=0;

		//printf("xd: %d \n", volume_adjust(30,100));

}


//16 bits
int detectNote( char*buff, int buff_size, int rate){
	//buff_size/=2;
	//esta interleaved, no agafem les mostres senars
	rate = 48000;
	int N = buff_size/BXS;
	int reduction = 2;
	double fft_mag[N/reduction];
	//adapt
	double adapt[N];	
	STYPE * sbuff = (STYPE *)&(buff[0]);
	for(int i=0; i<N; i+=2){ //TODO: en comptes de duplicar, reduir a la meitat! el rate tb
		double tmp = ((double)(sbuff[i])/32767)*(0.5-0.5*cos((2.0*PI*i)/(N-1)));
		adapt[i] = tmp;
		adapt[i+1] = tmp; //interleaved
		//printf("%lf\n%lf\n",i,adapt[i],adapt[i+1]);
	}
	//printf("\n\n\n");
	
	for(int k=0; k<N/reduction; ++k){
		double tmp = 0.0;
		double tmpi = 0.0;
		for(int i=0; i<N; ++i){
			double tmp2 = ((2*PI*k*i)/(double)N);
			tmp  += cos(tmp2)*adapt[i];
			tmpi += sin(tmp2)*adapt[i];
		}
		double d = tmp*tmp + tmpi*tmpi;
		if(d<0)d=0;
		fft_mag[k] = sqrt(d);
	}
	double max = 0;
	int pos=0;
	for(int i=1; i<N/reduction; ++i){
		if(fft_mag[i]>max){
		       	max=fft_mag[i];
			pos=i;
		}
	}
	int limit = (pos<10?pos:pos/2);
	double nom=pos*fft_mag[pos];
	double den=fft_mag[pos];
	for(int i=1; i<3;++i){
		nom += (pos-i)*fft_mag[pos-i];
		den += fft_mag[pos-i];

		nom += (pos+i)*fft_mag[pos+i];
		den += fft_mag[pos+i];
	}

	//printf("fft : %lf \n",nom/den);
	double result = (rate*nom*BXS)/(N*den);
	//double result = ((double)pos/(double)buff_size)*BXS*(double)rate;
	printf("result : %lf \n",result);
	/*int nota = 0;
	double ratio = (DO>result ? DO/result : result/DO);
	double mindist = fabs(ratio-(int)ratio);
	ratio = (DO_S>result ? DO_S/result : result/DO_S);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=DO_S;}
	ratio = (RE>result ? RE/result : result/RE);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=RE;}
	ratio = (RE_S>result ? RE_S/result : result/RE_S);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=RE_S;}
	ratio = (MI>result ? MI/result : result/MI);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=MI;}
	ratio = (FA>result ? FA/result : result/FA);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=FA;}
	ratio = (FA_S>result ? FA_S/result : result/FA_S);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=FA_S;}
	ratio = (SOL>result ? SOL/result : result/SOL);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=SOL;}
	ratio = (SOL_S>result ? SOL_S/result : result/SOL_S);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=SOL_S;}
	ratio = (LA>result ? LA/result : result/LA);
	if (abs(ratio-(int)ratio) < mindist) {mindist=fabs(ratio-(int)ratio);nota=LA;}
	printf("nota = %d \n",nota);*/		

}

void buff_volume_adjust(char * buff, int ini, int buff_size,double volume){
	//return c*volume/100;
	//double dc = 10.0;
	//double v = volume*volume*volume;
	short* sbuff = (short*)&(buff[0]);
	if(ini%2==1) printf("ini ha de ser un nombre parell");
	for(int i=0; i<buff_size/2; i+=1){
		sbuff[i]*=volume;
		/*int ivalue = (buff[i]<<8) + (buff[i+1]&0x0FF);
		int fvalue = ((double)(ivalue-32767)*v + 32767);
		if(fvalue>65535)fvalue=65535;
		buff[i] = fvalue>>8;
		buff[i+1] = fvalue&0x0FF;
		//printf("%d %d %d\n",i,ivalue,fvalue);*/
	}

	//unsigned short* sbuff = (unsigned short *)buff;
	//for(int i=0; i<buff_size/2;++i)printf("%d %d\n",i,sbuff[i]);

}

int inside_period=-1;
void synth(int f, int instr, char* buff, int buff_size,int rate){ //a 16 bits
	
	if(inside_period==-1)inside_period=0;

	double fd = (rate*2.0)/(double)f;
	fd/=2.0;//interleaved
	double att=0.01;
	//printf("period_samples = %lf \n", period_samples);
	short* sbuff = (short*)&(buff[0]);
	for(int i=0; i<buff_size/2;i+=2){
		////printf("%d\n",sizeof(short));
		short value = (sin((inside_period*2.0*PI)/fd))*32767*att;
		sbuff[i]=value;
		sbuff[i+1]=0; //interleaved
		++inside_period;
		if(inside_period>=fd)inside_period=0;
	}

	
	/*FILE* fid  = fopen("sortida2.txt", "w+");
	if(fid<=0)printf("lalala %s", strerror(errno));
	for(int i=0; i<buff_size;++i){
		fprintf(fid,"%d %d\n",i,buff[i]);
	}
	fclose(fid);*/

	
}

void distorsion(char* buff, int buff_size, double dis){
	//printf("%d -> %d\n",0xFFFE,vecToInt(0xFF,0xFF));
	//printf("@c %d \n",(int)buff);
	short* sbuff = (short*)&(buff[0]);
	//printf("@s %d \n",(int)sbuff);
	/*
	int max=0;
	for(int i=0; i<buff_size/BXS; ++i){
		if(sbuff[i]>0 && sbuff[i]>max) max = sbuff[i];
		else if (sbuff[i]<0 && sbuff[i]<-max) max = -sbuff[i];

	}*/

	for(int i=0; i<buff_size/BXS; ++i){
		int limit = -1639*dis + 32767;
		int r = rand()%100;
		r=0;
		if(r%2==0) r = -r;
		if (sbuff[i]>limit) sbuff[i]=limit+r;
		else if (sbuff[i]<-limit) sbuff[i]=-limit+r;

		//printf("%d %d\n",i,sbuff[i]);
	}
	/*
	for(int i=0; i<buff_size; i+=2){
		int value = vecToInt(buff[i+1],buff[i]);
		printf("%d %d\n",i/2,value);		
	}*/	
}






void printbuff(char* buff, int buff_size){

	STYPE* sbuff = (STYPE*)(&buff[0]);
	for(int i=0; i<buff_size/BXS;i+=2){
		STYPE s = sbuff[i];
		printf("%d\n%d\n",s,s);
	}
}
