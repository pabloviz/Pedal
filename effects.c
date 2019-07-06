#include "effects.h"
#include "utils.h"
#include "Diode.h"
#include <sys/time.h>
//new
//
double cachedvalue = -1.0;
long* intbuffer;
void echo(char* buff, int buff_size, char* savedbuff, int savedbuff_size,
	int savedbuff_pos, int bpm, double when, int howlong){

	/*if(when<0.0 || howlong*when*1000>SAVED_mSECONDS || bpm<0){
		printf("invalid parameters for echo %d %lf %d\n",bpm,when,howlong);
		return;
	}*/
	if(intbuffer==NULL) intbuffer = (long *)malloc((buff_size/2.0)*sizeof(long));
	if(cachedvalue==-1.0) cachedvalue = (double)savedbuff_size*60.0*1000.0;
	double bytes_per_beat = cachedvalue/((double)bpm*SAVED_mSECONDS);
	howlong = (howlong*bpm)/60;
	double bytes_back = bytes_per_beat * when;
	if (bytes_back == savedbuff_size) bytes_back -= (double)buff_size;
	STYPE* sbuff  = (STYPE*)(&buff[0]);
	STYPE* ssaved = (STYPE*)(&savedbuff[0]);
	double min = 0.01;
	double max = 1.0;	
	double power;
	//quadratic volume down
	double B = howlong-1.0;
	double l = (B!=0.0?((max-min)/(B*B)):1.0); //no hauria d'importar
	for(int it=1; it<=howlong; ++it){
		int index = savedbuff_pos - (int)bytes_back*it - buff_size;
		if (index < 0) index = savedbuff_size + index;
		index /= BXS;
		if(index%2==1)--index;
		power = (it!=1?(l*(it-howlong)*(it-howlong) + min):max);
		//power = 0.9;
		for (int i=0; i<buff_size/BXS; i+=2){
			STYPE old_value = ssaved[index+i];
			if(it==1) intbuffer[i/2] = sbuff[i];
			
			intbuffer[i/2] += old_value*power;
			if(it==howlong){
				if (intbuffer[i/2]>=MAXVALUE) sbuff[i] = MAXVALUE;
				else if(intbuffer[i/2]<=-MAXVALUE)sbuff[i]= -MAXVALUE;
				else sbuff[i] = intbuffer[i/2];
			}



			//if ((int)sbuff[i] + (int)old_value*power >= 32767) sbuff[i]=32767;
			//else if ((int)sbuff[i] + (int)old_value*power <= -32767) sbuff[i]=-32767;
			//else sbuff[i] = sbuff[i] + old_value*power;
		}
	}
}

void reverb(char* buff, int buff_size, char* savedbuff, int savedbuff_size,
	  int savedbuff_pos){
	
	STYPE* sbuff = (STYPE*)(&buff[0]);
	STYPE* ssaved = (STYPE*)(&savedbuff[0]);
	savedbuff_pos -= buff_size;

	/*int chunks_back = 150;
	savedbuff_pos -= buff_size;
	int index = savedbuff_pos - buff_size*chunks_back;
	if(index<0) index = savedbuff_size + index;
	if(index<0){
		printf("error in reverb\n");
		exit(0);
	}
	index /= 2;*/
	
	int nindexes = 13;
	int indexes[] = {
	//	2,4,6,8,10,12,14,16,18
		26,24,22,20,18,16,14,12,10,8,6,4,2
	};
	double multis[] = {
		0.05,0.07,0.09,0.11,0.14,0.15,0.24,0.29,0.35,0.41,
		0.49,0.53,0.55,0.57,0.59,0.61,0.63,0.65,0.67,0.69  
	};
	for(int i=0; i<nindexes; ++i){
		int tmp = indexes[i];
		tmp = savedbuff_pos - buff_size*tmp;
		if(tmp<0) tmp = savedbuff_size + tmp;
		indexes[i] = tmp/2;
	}


	for (int k=0; k<buff_size/BXS; k+=2){
		int tmp = 0;
		for (int i=0; i<nindexes; ++i){
			tmp += ssaved[indexes[i]+k]*multis[i];
		}
		sbuff[k] = sigadd(sbuff[k]*0.7,(STYPE)tmp);
	}


}


//16 bits
//char * detectbuff = (char *)-1;
//int detectbuff_size;
//int detectbuff_howmany=5;
//int counter=0;
int detectNote(/*char*buff,int buff_size,*/char* savedbuff,int savedbuff_size,int savedbuff_pos,int rate){
/*
	clock_t start, end;
	double cpu_time_used;
	start = clock();
	}
*/
	int factor = 128;
	int N = savedbuff_size/factor;
	N = N/(2*BXS);
	//printf("N: %d\n",N);
	int reduction = 15;
	double fft_mag[N/reduction];
	double adapt[N];
	STYPE * ssaved = (STYPE *)&(savedbuff[0]);
	int index = savedbuff_pos - savedbuff_size/factor;
	if(index<0) index = savedbuff_size + index;
	index/=BXS;
	STYPE op = 15;
	for(int i=0; i<N; i+=1){
		if(ssaved[index]>op)++op;
		double tmp = ((double)(ssaved[index])/32767)*(0.5-0.5*getCos((2.0*PI*i)/(N-1)));
		index+=2;
		if(index>=savedbuff_size/BXS)index=0;
		adapt[i] = tmp;
	}
	if(op==15) return -10;
	//printf("\n\n hola \n\n");
	//exit(0);
	double max=-100.0;
	int pos=0;
	double posd=0;
	int flag=0;

	for(int k=0; k<N/reduction; ++k){
		double tmp = 0.0;
		double tmpi = 0.0;
		for(int i=0; i<N; ++i){
			double tmp2 = ((2*PI*k*(i))/(double)N);
			tmp += getCos(tmp2)*adapt[i];
			tmpi += getSin(tmp2)*adapt[i];
			
		}
		double d = tmp*tmp + tmpi*tmpi;
		if(d<0)d=0;
		fft_mag[k] = /*sqrt(d)*/d;
		if(d > max){
			if(d>2.0){
				max=d;
				pos=k;
			}
		}else {
			posd = fft_mag[k-1]*(k-1) + max*(k-1) + d*k;
			posd /= fft_mag[k-1] + max + d;
			goto etiqueta;
		}

	}
	//printf("aixo no hauria de passar\n");
etiqueta:
	flag=0;
	double result = (rate*posd*BXS)/(N);
	//end = clock();
	//cpu_time_used = ((double)(end-start))/CLOCKS_PER_SEC;
	//printf("time: %lf\n",cpu_time_used);
	//printf("%d\n",(int)result);
	return (int)result;

}


int* inside_period=(int *)-1;
double* fd=(double *)-1;
void synth(int f, int instr, char* buff, int buff_size,int rate){ //a 16 bits
	
	if(inside_period==(int *)-1 || fd==(double*)-1){
		inside_period=(int *)malloc(sizeof(int)*4);
		fd = (double *)malloc(sizeof(double)*4);
	}

	int f_size=3;
	fd[0] = (double)rate/(double)f;
	fd[1] = fd[0]*=1.25;
	fd[2] = fd[0]*=1.5;
	/*for(int i=0; i<f_size;++i){
		fd[i] = (double)rate/(double)f;
	}*/

	//double fd = (rate*2.0)/(double)f;
	//fd/=2.0;//interleaved
	double att=0.05;
	
	short* sbuff = (short*)&(buff[0]);
	for(int i=0; i<buff_size/2;i+=2){
	
		for(int j=0; j<f_size;++j){
			double val = ((inside_period[j]*2.0*PI)/fd[j]);
			short value = getSin(val)*32767*att;
			if(j==0){
				sbuff[i]=value;
				sbuff[i+1]=0; //interleaved
			}else sbuff[i]+=value;
			++inside_period[j];
			if(inside_period[j]>=fd[j])inside_period[j]=0;
		}
	}
}



//molt optimitzable
void lala(char * buff, int buff_size, double dis, int tipo){
	STYPE* sbuff = (STYPE*) &(buff[0]);
	//dis = 0.95;
	//lowpass(buff, buff_size, tipo);
	for(int i=0; i<buff_size/BXS; i+=2){
		STYPE old = sbuff[i];
		STYPE new;
		if (old<0) new = getDiode(-old);
		else new = getDiode(old);
		if(old<0) sbuff[i] = old*(1-dis) - new*dis;
		else if (old>0) sbuff[i] = old*(1-dis) + new*dis;
		//else sbuff[i] = 0;
	}
}


void chorus(int f, char * buff, int buff_size, int rate){
;
}

int offset = 120;
int flag=1;
void flanger(char * buff, int buff_size, char * savedbuff, int savedbuff_size, int pos_in_savedbuff, int bpm){



}
STYPE last_low = 0;
void lowpass(char* buff, int buff_size, int ammount){
	double alfa = (double)ammount/10.0;
	//printf("%lf\n",alfa);
	STYPE* sbuff = (STYPE*) (&buff[0]);
	for (int i=0; i<buff_size/BXS; i+=2){
		sbuff[i] = sbuff[i]*(1.0-alfa) + last_low*alfa;
		last_low = sbuff[i];	
	}


}
