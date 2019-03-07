#include "effects.h"
#include "utils.h"
#include <fcntl.h>
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


/*
int howmany=-1;
char* echobuff = (char*)-1;
void echo(int period, char* buff, int buff_size,int ret, int layers){ //layers default = 1
		if(layers<=0) return;
		if(echobuff == (char*)-1 || howmany==-1){ //TODO cuidado malloc massa gran
			howmany = (ret*1000)/period;
			//printf("howmany : %d\n",howmany);
			echobuff_size = buff_size*howmany;
			//printf("echobuff_size: %d\n",echobuff_size);
			echobuff = (char*)malloc(echobuff_size);
			echobuff_index = 0;
			for(int i=0; i<echobuff_size; ++i) echobuff[i]=0;
		}
		//double power = 1.0/(double)layers+1;
		int tmp_howmany = howmany/layers;
		int index;
		char temp[buff_size];
		for(int i=0; i<buff_size;++i) temp[i] = buff[i];
		STYPE* sbuff = (STYPE*)&(buff[0]);
		STYPE* sebuff =(STYPE*)&(echobuff[0]);
		//double power_m=-BXS/(layers*buff_size); //en realitat el bxs esta dividinti
		//double power;
		for(int j=1; j<=layers; ++j)
		{
			double power = (double)layers/((layers+1)*j*4.0);
			//double power = 1.0/((layers+1)*j*1.0);
			if(j==layers) power = 0.05;
			int samples_back = tmp_howmany*j*buff_size;//quantes mostres enrere (8bits)
			if (samples_back%2==1) printf("ei\n");
			if (samples_back>howmany*buff_size){
				printf("error\n");
				return;
			}
			//samples_back/=BXS;//mostres enrere en 16 bits
			//printf("shorts back %d\n",samples_back);
			for(int i=0; i<buff_size/BXS; i+=1){
				int actualindex = echobuff_index*buff_size +BXS*i;//a nivell de byte
				int ecoindex = actualindex - samples_back;
				//printf("index %d\n",index);
				if (ecoindex<0)ecoindex = echobuff_size+ecoindex;
				STYPE new_value = sbuff[i];
				//power = power_m*(l*buff_size/BXS + i)+1.0;
				STYPE old_value = sebuff[ecoindex/BXS]*power;
				//if(old_value<50 && old_value>-50)old_value=0;
				//printf("i: %d, index:%d\n",echobuff_index*buff_size/BXS + i,index/BXS);
				if(((int)new_value + (int)old_value )>32767) sbuff[i]=32767;
				else if (((int)new_value + (int)old_value)<-32767) sbuff[i] = -32767;
				
				else sbuff[i] = new_value+old_value;
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
			echobuff[echobuff_index*buff_size +i] = temp[i];
		}
		++echobuff_index;
		if(echobuff_index >= howmany)echobuff_index=0;

		//printf("xd: %d \n", volume_adjust(30,100));

}*/
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

	clock_t start, end;
	double cpu_time_used;
	start = clock();

	//STYPE * sbuff = (STYPE *)&(buff[0]);
	/*
	for(int i=0; i<buff_size/BXS;++i)if(sbuff[i]>50) goto jeje;
	return -10;
	jeje:
	
	if(detectbuff==(char*)-1){
		counter = 0;
		detectbuff_size = detectbuff_howmany*buff_size/2; 
		//diria que el 2 es per l'interleaved
		detectbuff = (char *)malloc(detectbuff_size);
	}
	if(counter==detectbuff_howmany){ counter=0;}
	else{
		for(int i=0; i<buff_size; i+=4) {
			int index = (detectbuff_size/detectbuff_howmany)*counter +i/2;
			detectbuff[index]=buff[i];
			detectbuff[index+1]=buff[i+1];
		}
		++counter;
		return -1;
	}*/
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
	printf("aixo no hauria de passar\n");
etiqueta:
	flag=0;
	double result = (rate*posd*BXS)/(N);
	end = clock();
	cpu_time_used = ((double)(end-start))/CLOCKS_PER_SEC;
	//printf("time: %lf\n",cpu_time_used);
	//printf("%d\n",(int)result);
	return (int)result;

}

void buff_volume_adjust(char * buff, int ini, int buff_size,double volume){
	//return c*volume/100;
	//double dc = 10.0;
	//double v = volume*volume*volume;
	short* sbuff = (short*)&(buff[0]);
	if(ini%2==1) printf("ini ha de ser un nombre parell");
	for(int i=0; i<buff_size/2; i+=1){
		int new = (int)sbuff[i]*volume;
		if(new>32767)new=32767;
		else if (new<-32767)new=-32767;
		sbuff[i]=(STYPE)new;
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

	
	/*FILE* fid  = fopen("sortida2.txt", "w+");
	if(fid<=0)printf("lalala %s", strerror(errno));
	for(int i=0; i<buff_size;++i){
		fprintf(fid,"%d %d\n",i,buff[i]);
	}
	fclose(fid);*/

	
}
//int pos = -1;
//int posn = -1;
//int pablito=0;
//
//
//int pendent =0;
int * minmaxs = (int *)-1;
int minmaxsindex;
void lala(char * buff, int buff_size, double dis, int tipo){


	short * sbuff = (short*)&(buff[0]);
	int pendent = (sbuff[2] > sbuff[0]);
	int new_pendent;
	if(minmaxs==(int*)-1) minmaxs = (int *)malloc(sizeof(int)*buff_size/BXS);
	minmaxsindex=0;
	for(int i=2; i<buff_size/BXS; i+=2){
		new_pendent = (sbuff[i] > sbuff[i-2]);
		if (new_pendent != pendent) {
			//printf("max at i = %d / i/2 = %d => %d\n",i,i/2,sbuff[i]);
			minmaxs[minmaxsindex] = i;
			++minmaxsindex;				
		}
		pendent = new_pendent;
		//printf("i: %d b: %d\n",i,sbuff[i]);
	}
	for(int i=0; i<minmaxsindex; ++i){
		int index = minmaxs[i];
		//printf("index %d\n",index);
		STYPE v = sbuff[index];
		STYPE mask = (STYPE)(-(v<0));
		int limit = ((v^mask)-mask)/128 + 1;
		//printf("limit %d\n",limit);

		int tmp=0;
		if(i>0)tmp=minmaxs[i-1];
		for(int j=index; j>0 && j>tmp && j>index-limit; j-=2){
			sbuff[j]=0;//posarho a 0 fot una distorsio de la parra
		}
		tmp = buff_size/BXS;
		if(i<minmaxsindex-1)tmp=minmaxs[i+1];
		for(int j=index; j<buff_size/BXS && j<tmp && j<index+limit;j+=2){
			sbuff[j]=0;
		}	
	}


	//exit(0);
}




void distorsion(char* buff, int buff_size, double dis, int tipo){
	//printf("%d -> %d\n",0xFFFE,vecToInt(0xFF,0xFF));
	//printf("@c %d \n",(int)buff);
	short* sbuff = (short*)&(buff[0]);

	double den = (dis/4.0) + 1.0;
	int limit = 32767.0/den;
	//printf("limit: %d\n",limit);
	//return;
	int pos = -1;
	int posn = -1;
	int a,b;
	double l;
	//int tipo = 2;//0:linear, 1:quadratic, 2:cubic, 3:atenuation
	for(int i=0; i<buff_size/BXS; i+=2){
		if (sbuff[i]>limit && pos==-1) {//obrim
			if(tipo!=0)pos=i;
			else sbuff[i]=limit;
		}else if ((i==(buff_size/BXS)-2 || sbuff[i]<limit) && pos!=-1){//tanquem
			if(tipo==1){
				quadunion(sbuff,buff_size,pos,i,limit,5);
			}else if(tipo==2){
				if(pos==0){
					int lon = (i-pos)/2;
					if(lon%2!=0)++lon;
					quadunion(sbuff,buff_size,0,lon,limit,5);
					quadunion(sbuff,buff_size,lon,i,limit,-5);
				}
				if(i==(buff_size/BXS)-2){
					quadunion(sbuff,buff_size,pos,i,limit,-5);
				}else if(pos!=0){
					int lon = (i-pos)/3;
					if (lon%2!=0)++lon;
					int l1 = pos + lon;
					int l2 = l1 + lon;
					quadunion(sbuff,buff_size,pos,l1,limit,-5);
					quadunion(sbuff,buff_size,l1,l2,limit,5);
					quadunion(sbuff,buff_size,l2,i,limit,-5);
				}
			}else if(tipo==3){
				for(int j=pos;j<=i;j+=2)sbuff[j]=((sbuff[j]-limit)*0.4)+limit;
			}
			pos = -1;	
		}else if (sbuff[i]<-limit && posn==-1){//obrim
			if(tipo!=0)posn=i;
			else sbuff[i]=-limit;
		}else if ((i==(buff_size/BXS)-2 ||sbuff[i]>-limit) && posn!=-1){//tanquem
			if(tipo==1){
				quadunion(sbuff,buff_size,posn,i,-limit,-5);
			}
			else if(tipo==2){
				if(posn==0){
					int lon = (i-posn)/2;
					if(lon%2!=0)++lon;
					quadunion(sbuff,buff_size,0,lon,-limit,-5);
					quadunion(sbuff,buff_size,lon,i,-limit,5);
				}else if(i==(buff_size/BXS)-2){
					quadunion(sbuff,buff_size,posn,i,-limit,5);
				}else{
					int lon = (i-posn)/3;
					if(lon%2!=0)++lon;
					int l1 = posn +lon;
					int l2 = l1 + lon;
					quadunion(sbuff,buff_size,posn,l1,-limit,5);
					quadunion(sbuff,buff_size,l1,l2,-limit,-5);
					quadunion(sbuff,buff_size,l2,i,-limit,5);
				}
			}else if(tipo==3){
				for(int j=pos;j<=i;j+=2)sbuff[j]=((sbuff[j]+limit)*0.5)-limit;

			}

			posn=-1;
		}
		//printf("%d %d\n",i,sbuff[i]);
	}
	/*
	for(int i=0; i<buff_size; i+=2){
		int value = vecToInt(buff[i+1],buff[i]);
		printf("%d %d\n",i/2,value);		
	}*/	
}


void chorus(int f, char * buff, int buff_size, int rate){
;
}

int offset = 120;
int flag=1;
void flanger(char * buff, int buff_size, char * savedbuff, int savedbuff_size, int pos_in_savedbuff, int bpm){



}

void printbuff(char* buff, int buff_size){

	STYPE* sbuff = (STYPE*)(&buff[0]);
	for(int i=0; i<buff_size/BXS;i+=2){
		STYPE s = sbuff[i];
		printf("%d\n",s);
	}
	//printf("\n\n\naaaaaaaaaaa\n\n\n");
}
