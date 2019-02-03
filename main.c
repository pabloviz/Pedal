
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<errno.h>
// #include "./include/alsa/asoundlib.h"
#include <alsa/asoundlib.h>
#include <math.h>
#include "notes.h"

#define MYTYPE char
#define PI 3.14159265

static char *device = "default";

snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
int rate = 44100;
int channels = 1;
int seconds = 5;

void init();

int howmany=-1;
MYTYPE * echobuff = -1;
void echo(int loop, int period,int read, MYTYPE* buff, int buff_size, int ret, int layers);//layers default = 1

void synth(int f, int instr, MYTYPE* buff, int buff_size);

int detectNote( MYTYPE* buff, int buff_size);

int main(void)
{
	
	//PCM = pulse code modulation
	int err;
	snd_pcm_sframes_t frames;
	//init();
	//open PCM
	 err = snd_pcm_open(&handle,device,SND_PCM_STREAM_PLAYBACK,0);
	if(err<0){
		printf("Playback open error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//allocate parameteres object, default values
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle,params);
	//set params
	err = snd_pcm_hw_params_set_access(handle,params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if(err<0) printf("Cant set inreleaved mode. %s \n", strerror(errno));

	err = snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_S16_LE);
	if(err<0) printf("Cant set format. %s \n", strerror(errno));
	
	err = snd_pcm_hw_params_set_channels(handle,params,channels);
	if(err<0) printf("Cant set channels number. %s \n", strerror(errno));
	
	err = snd_pcm_hw_params_set_rate_near(handle,params,&rate,0);
	if(err<0) printf("Cant set rate. %s \n", strerror(errno));
	//write params
	err = snd_pcm_hw_params(handle,params);
	if(err<0) printf("Cant write params. %s \n", strerror(errno));
	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	int buff_size = frames * channels *2; //el 2 es per el "sample size"
	printf("buff size is %i \n" , buff_size);
	MYTYPE * buff= (MYTYPE *)malloc(buff_size);
	int tmp = 0;
	snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
	printf("tmp = : %d\n",tmp);


	int loops = (seconds*1000000)/tmp;
	printf("loops : %d\n",loops);
	for (int l =0; l<=loops; ++l){
		//read
		err = read(0,buff,buff_size);
		if(err<0) printf("error de lectura. %s \n", strerror(errno));
		
		//echo(l,tmp,err,buff,buff_size,500,2);
		//synth(440,0,buff,buff_size);	
		detectNote(buff,buff_size);
		//write
		err = snd_pcm_writei(handle,buff,frames);
		if(err<0) {
			printf("bon dia. %d\n", errno);
			snd_pcm_prepare(handle);
		}
		//if(err > 0 && err < (long)sizeof(buff)) printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buff),err);
	}
	return 0;
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	
	free(buff);
	free(echobuff);
	return 0;
}


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
int inside_period=-1;
void synth(int f, int instr, MYTYPE* buff, int buff_size){
	
	if(inside_period==-1)inside_period=0;
	
	
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
	double period_samples = (rate*2.0)/(double)f;
	//printf("period_samples = %lf \n", period_samples);
	for(int i=0; i<buff_size;++i){
		double value = (sin(inside_period*2.0*PI/(double)period_samples))*128 + 127;
		buff[i] = (char) value;
		++inside_period;
		if(inside_period>=period_samples)inside_period=0;
	}

	
	FILE* fd  = fopen("sortida2.txt", "w+");
	if(fd<=0)printf("lalala %s", strerror(errno));
	for(int i=0; i<buff_size;++i){
		fprintf(fd,"%d %d\n",i,buff[i]);
	}
	fclose(fd);

	
}

int detectNote( MYTYPE*buff, int buff_size){
	
	int mean=0;
	int counter=0;
	int longitude = 0;
	
	int creixent = 1;
	int firsttime=1;	
	for(int i=1; i<buff_size;++i){
		if(creixent){
			if(buff[i]>=buff[i-1]) ++longitude;
		        else{
		       		creixent = 0;
				if(!firsttime){
					mean+=longitude;
					counter+=1;
				}else firsttime=0;
				longitude=0;
		        }
		}else{
			if(buff[i]<=buff[i-1]) ++longitude;
			else{
				creixent=1;
				if(!firsttime){
					mean+=longitude;
					counter+=1;
				}else firsttime=0;
				longitude=0;
			}
		}	
	}
	double d_max = rate/((double)mean/(double)counter); //s'ha simplificat un 2 adalt i abaix
	printf("prediccio: %lf \n", d_max);
	return 0;

}

void init(){

	//open PCM
	int err = snd_pcm_open(&handle,device,SND_PCM_STREAM_PLAYBACK,0);
	if(err<0){
		printf("Playback open error: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//allocate parameteres object, default values
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle,params);
	//set params
	err = snd_pcm_hw_params_set_access(handle,params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if(err<0) printf("Cant set inreleaved mode. %s \n", strerror(errno));

	err = snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_S16_LE);
	if(err<0) printf("Cant set format. %s \n", strerror(errno));
	
	err = snd_pcm_hw_params_set_channels(handle,params,channels);
	if(err<0) printf("Cant set channels number. %s \n", strerror(errno));
	
	err = snd_pcm_hw_params_set_rate_near(handle,params,&rate,0);
	if(err<0) printf("Cant set rate. %s \n", strerror(errno));
	//write params
	err = snd_pcm_hw_params(handle,params);
	if(err<0) printf("Cant write params. %s \n", strerror(errno));

	//?
}
