
STYPE * detectbuff = (STYPE *)-1;
int detectbuff_size;
int detectbuff_howmany=10;
int counter=0;
int detectNote( char*buff, int buff_size, int rate){

	clock_t start, end;
	double cpu_time_used;
	start = clock();

	STYPE * sbuff = (STYPE *)&(buff[0]);
	for(int i=0; i<buff_size/BXS;++i){
		if(sbuff[i]>50) goto jeje;
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


	int N = detectbuff_size/BXS;
	//printf("%d\n",N);
	//return;
	int reduction = 15;
	double fft_mag[N/reduction];
	//adapt
	double adapt[N];
	//double blank[N];
	//printf("N: %d\n",N);
	printf("hola\n");	
	for(int i=0; i<N; i+=1){
		printf("%d\n",detectbuff[i]);
		double tmp = ((double)(detectbuff[i])/32767)*(0.5-0.5*cos((2.0*PI*i)/(2*N-1)));
		adapt[i] = tmp;
		//blank[i]=0;
	}
	return 0;
	//printf("\n\n\n\n");			
	//Fft_transform(adapt,blank,N);	
	//goto etiqueta;
	
	for(int k=0; k<N/reduction; ++k){
		double tmp = 0.0;
		double tmpi = 0.0;
		for(int i=0; i<N; ++i){
			double tmp2 = ((2*PI*k*(i))/(double)N);
			/*
			tmp  += cos(tmp2)*adapt[i];
			tmpi += sin(tmp2)*adapt[i];
			*/
			tmp += getCos(tmp2)*adapt[i];
			tmpi += getSin(tmp2)*adapt[i];
		}
		double d = tmp*tmp + tmpi*tmpi;
		if(d<0)d=0;
		fft_mag[k] = sqrt(d);
	}
//etiqueta:
	//reduction=15;
	double max = -100.0;
	int pos=0;
	for(int i=0; i<N/reduction; ++i){
		printf("%lf\n",fft_mag[i]);
		if(fft_mag[i]>max){
		//if(adapt[i]>max){
		       	if(fft_mag[i]>3){
			//if(adapt[i]>1){
				max=fft_mag[i];
				//max = adapt[i];
				pos=i;
			}
		}//else if(max!=-100.0)break;
	}
	
	int limit = (pos<10?pos:pos/5);
	double nom=pos*fft_mag[pos];
	double den=fft_mag[pos];
	/*for(int i=1; i//<limit;++i){
		nom += (pos-i)*fft_mag[pos-i];
		den += fft_mag[pos-i];

		nom += (pos+i)*fft_mag[pos+i];
		den += fft_mag[pos+i];
	}*/
	nom = pos;
	den = 1;
	double result = (rate*nom*BXS)/(N*den);
	end = clock();
	cpu_time_used = ((double)(end-start))/CLOCKS_PER_SEC;
	printf("time: %lf\n",cpu_time_used);
	printf("%d\n",(int)result);
	return (int)result;


}

//del main
//

/*
void echo(int loop, int period,int read, MYTYPE* buff, int buff_size,int ret, int layers){ //layers default = 1
		if(layers<=0) layers=1;
		if(echobuff == -1 || howmany==-1){
			howmany = (ret*1000)/period;
			echobuff = (MYTYPE*)malloc(buff_size*howmany);
		}
		double power = 1.0/(double)layers;
		int tmp_howmany = howmany/((layers==1)+(layers-1));
		//soc un geni de les caches.
		for(int j=1; j<layers; ++j){
			int index = tmp_howmany*j;	
			for(int i=0; i<read; ++i){
				char new = buff[i];
				if (loop>=index){
					MYTYPE old = echobuff[((loop-index)%howmany)*buff_size+i];
					buff[i] = buff[i] + old;
				}
				if(j==1)echobuff[(loop%howmany)*buff_size+i] = new;
			}
		}	
}
*/

/*
int inside_period=-1;
void synth(int f, int instr, MYTYPE* buff, int buff_size){
	
	if(inside_period==-1)inside_period=0;
	
*/	
	/* deixo aixo lineal perque sona guay en plan 8 bits
	int period_samples = (rate*2)/f;
	int quarter = period_samples/4;
	float m = (255.0-127.0)/(float)quarter;

	for(int i=0; i<buff_size;++i){ //tinc un dibuix molt maco a la llibreta chateauform
		if(inside_period<quarter) 
			buff[i]=127+m*inside_period;

		if(inside_period>=quarter && inside_period<3*quarter) 
			buff[i]=255-m*(inside_period-quarter);

		if(inside_period>=3*quarter && i<=period_samples) 
			buff[i]= m*(inside_period-3*quarter);
		
		++inside_period;
		if(inside_period>=period_samples)inside_period=0;	
	}*/


/*
	double fd = (rate*2.0)/(double)f;
	//printf("period_samples = %lf \n", period_samples);
	for(int i=0; i<buff_size;++i){
		double value = (sin((inside_period*2.0*PI)/fd))*128 + 127;
		buff[i] = (char) value;
		++inside_period;
		if(inside_period>=fd)inside_period=0;
	}*/

	
	/*FILE* fid  = fopen("sortida2.txt", "w+");
	if(fid<=0)printf("lalala %s", strerror(errno));
	for(int i=0; i<buff_size;++i){
		fprintf(fid,"%d %d\n",i,buff[i]);
	}
	fclose(fid);*/
/*
	
}*/


/*
int detectNote( MYTYPE*buff, int buff_size){ //TODO TE UN GRAN PROBLEMA QUAN FD NO DIVIDEIX A BUFF_SIZE
	
	double fft_real[buff_size];
	double fft_img[buff_size];
	//adapt
	double adapt[buff_size];  //TODO posar aixo en el bucle intern i aixi m'estalvio bucles extres?
	for(int i=0; i<buff_size; ++i){
		adapt[i] = ((double)buff[i]-127.0)/128.0;
	}

	for(int k=0; k<buff_size; ++k){
		double tmp = 0.0;
		double tmpi = 0.0;
		for(int i=0; i<buff_size; ++i){
			//inputreal[i] = cos(((double)buff[i]*2*PI*i)/buff_size);
			//inputimg[i]  = sin(((double)buff[i]*2*PI*i)/buff_size);
			double tmp2 = ((2*PI*k*i)/(double)buff_size)*adapt[i];
			tmp += cos(tmp2);
			tmpi -= sin(tmp2);
		}
		fft_real[k]=tmp/buff_size;
		fft_img[k]=tmpi/buff_size;
	}
	//double fft_final[buff_size];
	//for(int i=0; i<buff_sizze

	//Fft_transform(inputreal,inputimg,buff_size);
	double max = 0;
	int pos=0;
	for(int i=1; i<buff_size; ++i){
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
	int nota = 0;
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
	printf("nota = %d \n",nota);		

}*/

