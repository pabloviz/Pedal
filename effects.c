#include "effects.h"
//new
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
		double power = 1.0/(double)layers+1;
		int tmp_howmany = howmany/layers;
		int index;

		char temp[buff_size];
		//soc un geni de les caches.
		for(int j=1; j<=layers; ++j){
			int samples_back = tmp_howmany*j*buff_size;//quantes mostres enrere
			if (samples_back%2==1) printf("ei\n");
			if (samples_back>howmany*buff_size){
				printf("error\n");
				return;
			}
			//printf("samples back: %d\n",samples_back);
			for(int i=0; i<buff_size; ++i){
				//char new = buff[i];
				if(j==1)temp[i]=buff[i];
				//if(j==1) echobuff[echobuff_index*buff_size + i]
				int index = (echobuff_index*buff_size + i) - samples_back;
				if (index<0) {
					index = echobuff_size+index;
				}
				buff[i] = buff[i] + echobuff[index];
				//if(i==10)printf("im at %d, final index is %d\n",echobuff_index*buff_size + i,index);
				//if(j==1) echobuff[echobuff_index*buff_size + i] = new;
			}
		}
		//actualitzar echo buffer
		for(int i=0;i<buff_size;++i) {
			echobuff[echobuff_index*buff_size +i] = temp[i]*(temp[i]>1);
		}
		++echobuff_index;
		if(echobuff_index >= howmany)echobuff_index=0;

		//printf("xd: %d \n", volume_adjust(30,100));

}
//16 bits
int detectNote( char*buff, int buff_size, int rate){ //TODO TE UN GRAN PROBLEMA QUAN FD NO DIVIDEIX A buff_size
	buff_size/=10;
	
	double fft_real[buff_size/2];
	double fft_img[buff_size/2];
	//adapt
	double adapt[buff_size/2];  //TODO posar aixo en el bucle intern i aixi m'estalvio bucles extres?
	for(int i=0; i<buff_size; i+=2){
		int upper = (int)buff[i];
		int lower = (int)buff[i+1];

		int tmp = (upper<<8) + (lower&0x0FF);
		//printf("tmp : %d\n",tmp);
		//printf("upper: %d; lower: %d\n",upper,lower);
		adapt[i/2] = (double)((tmp-32767))/32768.0;
		//aplicar hann
		//adapt[i] *= 0.53836 - 0.46164*cos((2*PI*i)/buff_size-1);
		//printf("%d %lf\n",i/2,adapt[i/2]);
	}
	//return;

	for(int k=0; k<buff_size/2; ++k){
		double tmp = 0.0;
		double tmpi = 0.0;
		for(int i=0; i<buff_size/2; ++i){
			//inputreal[i] = cos(((double)buff[i]*2*PI*i)/buff_size);
			//inputimg[i]  = sin(((double)buff[i]*2*PI*i)/buff_size);
			double tmp2 = ((2*PI*k*i)/(double)buff_size/2)*adapt[i];
			tmp += cos(tmp2);
			tmpi -= sin(tmp2);
		}
		fft_real[k]=tmp/(buff_size/2);
		fft_img[k]=tmpi/(buff_size/2);
	}
	//double fft_final[buff_size];
	//for(int i=0; i<buff_sizze

	//Fft_transform(inputreal,inputimg,buff_size);
	double max = 0;
	int pos=0;
	for(int i=1; i<buff_size/2; ++i){
		double f = sqrt(fft_real[i]*fft_real[i]+fft_img[i]*fft_img[i]);
		if(f>max){
		       	max=f;
			pos=i;
		}
	//	printf("i: %d, real: %lf, img: %lf, final :%lf \n",i,fft_real[i],fft_img[i],f);
	}
	//printf("fft : %d \n",pos);
	double result = ((double)pos/(double)buff_size)*2.0*(double)rate;
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


char volume_adjust(char * buff, int ini, int buff_size,double volume){
	//return c*volume/100;
	//double dc = 10.0;
	double v = volume*volume*volume;
	if(ini%2==1) printf("ini ha de ser un nombre parell");
	for(int i=0; i<buff_size; i+=2){
		int ivalue = (buff[i]<<8) + (buff[i+1]&0x0FF);
		int fvalue = ((double)(ivalue-32767)*v + 32767);
		if(fvalue>65535)fvalue=65535;
		buff[i] = fvalue>>8;
		buff[i+1] = fvalue&0x0FF;
		//printf("%d %d %d\n",i,ivalue,fvalue);
	}

	//unsigned short* sbuff = (unsigned short *)buff;
	//for(int i=0; i<buff_size/2;++i)printf("%d %d\n",i,sbuff[i]);

}

int inside_period=-1;
void synth(int f, int instr, char* buff, int buff_size,int rate){ //a 16 bits
	
	if(inside_period==-1)inside_period=0;

	double fd = (rate*2.0)/(double)f;
	double att=0.001;
	//printf("period_samples = %lf \n", period_samples);
	for(int i=0; i<buff_size;i+=2){
		//printf("%d\n",sizeof(short));
		int value = (sin((inside_period*2.0*PI)/fd))*32768*att + 32766;
		//printf("%d %d\n",i/2,value);
		int upper = value>>8;
		int lower = value&0x0FF;
		buff[i] = upper;
		buff[i+1] = lower;
		//printf("%d %d\n",upper,lower);
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
void printbuff(char* buff, int buff_size){

	for(int i=0; i<buff_size;i+=2){
		short s = buff[i]<<8 + buff[i+1];
		printf("%d %d\n",i,s);
	}
}
