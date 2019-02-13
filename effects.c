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
		for(int i=0; i<buff_size;++i) temp[i] = buff[i];
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
			echobuff[echobuff_index*buff_size +i] = temp[i]/* (temp[i]>50 || temp[i]<-50)*/;
		}
		++echobuff_index;
		if(echobuff_index >= howmany)echobuff_index=0;

		//printf("xd: %d \n", volume_adjust(30,100));

}


//16 bits
STYPE * detectbuff = (STYPE *)-1;
int detectbuff_size;
int detectbuff_howmany=10;
int counter=0;
int detectNote( char*buff, int buff_size, int rate){

	STYPE * sbuff = (STYPE *)&(buff[0]);
	for(int i=0; i<buff_size/BXS;++i){
		if(sbuff[i]>100) goto jeje;
	}
	return -10;
	jeje:
	if(detectbuff==(STYPE*)-1){
		detectbuff_size = detectbuff_howmany*buff_size/(2); //diria que el 2 es per l'interleaved
		detectbuff = (STYPE *)malloc(detectbuff_size);
	}
	if(counter==detectbuff_howmany){ counter=0;}
	else{
		for(int i=0; i<buff_size/BXS; i+=2){
			detectbuff[i/2 + counter*buff_size/(BXS*2)]=sbuff[i];
		}
		++counter;
		return -1;
	}
	
	//exit(0);
	//buff_size/=2;
	//esta interleaved, no agafem les mostres senars
	//rate = 414000;
	//int N = buff_size/BXS;
	int N = detectbuff_size/BXS;
	int reduction = 15;
	double fft_mag[N/reduction];
	//adapt
	double adapt[N];
	//printf("N: %d\n",N);	
	for(int i=0; i<N; i+=1){
		//printf("%d\n",detectbuff[i]);
		double tmp = ((double)(detectbuff[i])/32767)*(0.5-0.5*cos((2.0*PI*i)/(2*N-1)));
		adapt[i] = tmp;
		//adapt[i+1] = tmp; //interleaved
		//printf("%lf\n",adapt[i]);
	}	
	//printf("\n\nbuscam\n\n");
	//exit(0);
	//return;
	
	for(int k=0; k<N/reduction; ++k){
		double tmp = 0.0;
		double tmpi = 0.0;
		for(int i=0; i<N; ++i){
			double tmp2 = ((2*PI*k*(i))/(double)N);
			tmp  += cos(tmp2)*adapt[i];
			tmpi += sin(tmp2)*adapt[i];
		}
		double d = tmp*tmp + tmpi*tmpi;
		if(d<0)d=0;
		fft_mag[k] = sqrt(d);
	}
	double max = -100.0;
	int pos=0;
	for(int i=2; i<N/reduction; ++i){
		//printf("%lf\n",fft_mag[i]);
		if(fft_mag[i]>max){
		       	if(fft_mag[i]>1){
				max=fft_mag[i];
				pos=i;
			}
		}else break;
	}
	//printf("\n\n\n\n");
	
	int limit = (pos<10?pos:pos);
	double nom=pos*fft_mag[pos];
	double den=fft_mag[pos];
	for(int i=1; i<limit;++i){
		nom += (pos-i)*fft_mag[pos-i];
		den += fft_mag[pos-i];

		nom += (pos+i)*fft_mag[pos+i];
		den += fft_mag[pos+i];
	}
	nom = pos;
	den = 1;
	//printf("%lf\n",nom/den);
	//double nom = (double)pos;
	//double den = 1.0;

	//printf("fft : %lf \n",nom/den);
	double result = (rate*nom*BXS)/(N*den);
	//return (int)result;
	//double result = ((double)pos/(double)buff_size)*BXS*(double)rate;
	//printf("result : %lf \n",result);
	return (int)result;
//	exit(0);
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
//int pos = -1;
//int posn = -1;
//int pablito=0;
void distorsion(char* buff, int buff_size, double dis, int tipo){
	//printf("%d -> %d\n",0xFFFE,vecToInt(0xFF,0xFF));
	//printf("@c %d \n",(int)buff);
	short* sbuff = (short*)&(buff[0]);
	/*
	if(pablito==0){
		for(int i=0; i<buff_size/BXS -50;i+=2){
			sbuff[i] = -1000;
		}
		for(int i=buff_size/BXS -50; i<buff_size/BXS; i+=2){
			sbuff[i] = -1500;
		}
	}else if (pablito==1){
		for(int i=0; i<50;++i){
			sbuff[i] = -2000;
		}for(int i=50; i<buff_size/BXS; i+=2){
			sbuff[i] = -1000;
		}
	}
	++pablito;*/	
	//printf("@s %d \n",(int)sbuff);
	/*
	int max=0;
	for(int i=0; i<buff_size/BXS; ++i){
		if(sbuff[i]>0 && sbuff[i]>max) max = sbuff[i];
		else if (sbuff[i]<0 && sbuff[i]<-max) max = -sbuff[i];

	}*/
	int limit = -1639*dis + 32767;
	//printf("limit: %d\n",limit);
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
				for(int j=pos;j<=i;j+=2)sbuff[j]=((sbuff[j]-limit)*0.5)+limit;
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






void printbuff(char* buff, int buff_size){

	STYPE* sbuff = (STYPE*)(&buff[0]);
	for(int i=0; i<buff_size/BXS;i+=2){
		STYPE s = sbuff[i];
		printf("%d\n",s);
	}
	printf("\n\n\naaaaaaaaaaa\n\n\n");
}
