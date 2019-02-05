#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<errno.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
//#include "openusb.h"
#include <libusb-1.0/libusb.h>
#include "notes.h"
#include "fft.h"

#define MYTYPE char
#define PI 3.14159265
#define ISOSIZE 180

static char *device = "default";

snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
int rate = 44100;
int channels = 1;
int seconds = 5;
snd_pcm_sframes_t frames;
char* buff;
int buff_size;

void DeviceScan(libusb_context *ctx, libusb_device **devs);
void init_output();
void iniusblib(libusb_context * ctx);
libusb_device_handle* selectDevice(libusb_context *ctx, int vid, int pid, int interface, int alt_setting);
void iniTransmission(libusb_device_handle * dev_handle, struct libusb_transfer* trans);

int howmany=-1;
MYTYPE * echobuff = -1;
void echo(int loop, int period,int read, MYTYPE* buff, int buff_size, int ret, int layers);//layers default = 1
void synth(int f, int instr, MYTYPE* buff, int buff_size);

int detectNote( MYTYPE* buff, int buff_size);

int temporalbuffer_size;
char* temporalbuffer;
int tempbuf_index=0;
int lel = 1;

int buffindex=0;
static void callback(struct libusb_transfer* transfer){
	
	
	if (transfer->status != LIBUSB_TRANSFER_COMPLETED){
		printf("Transfer not completed. status = %d \n",transfer->status);
		return;
	}
	//printf("Good transmision \n");
	if (transfer->type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS){ //les iso pden tenir errors puntuals
		for(int i=0; i<transfer->num_iso_packets; ++i){
			struct libusb_iso_packet_descriptor *packet = &transfer->iso_packet_desc[i];
			if (packet->status != LIBUSB_TRANSFER_COMPLETED){
				printf("Error in packet number %d of %d \n",i,transfer->num_iso_packets);
				return;
			}
		}
	}
	//printf("Transfer length: %d, Actual length: %d\n",transfer->length, transfer->actual_length);	
	
	int err;
	//char bu[transfer->length];	
	//read(0,bu,transfer->length);
	
	for(int i=0; i<transfer->length;++i){ //TODO: per accelerar..en realitat aquest buffer jo el tinc
		
		if(transfer->buffer[i]<254){
			buff[buffindex] = transfer->buffer[i];
			++buffindex;
		}
		//buff[buffindex] = bu[i];
		//buffindex=1024;
		if(buffindex>=1024){
			err = snd_pcm_writei(handle,buff,frames);
			if(err<0){
				printf("error snd, prepare \n");
				snd_pcm_prepare(handle);
			}
			//printf("sent! \n");
			buffindex=0;
		}
	}
	libusb_submit_transfer(transfer);
}


static void callback_old(struct libusb_transfer* transfer){

	if (transfer->status != LIBUSB_TRANSFER_COMPLETED){
		printf("Transfer not completed. status = %d \n",transfer->status);
		return;
	}
	//printf("Good transmision \n");
	if (transfer->type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS){ //les isocrones poden tenir errors als packets
		for(int i=0; i<transfer->num_iso_packets; ++i){
			struct libusb_iso_packet_descriptor *packet = &transfer->iso_packet_desc[i];

			if (packet->status != LIBUSB_TRANSFER_COMPLETED){
				printf("Error in packet number %d of %d \n",i,transfer->num_iso_packets);
				return;
			}
		}
	}
	//printf("Transfer length: %d, Actual length: %d\n",transfer->length, transfer->actual_length);	

	for(int i=0; i<transfer->length; ++i){


		temporalbuffer[tempbuf_index]=transfer->buffer[i];
		++tempbuf_index;
		if(lel){
		int x = transfer->buffer[i];
		printf("%d ",transfer->buffer[i]);
		if(x<100) printf(" ");
		if(x<10) printf(" ");
		if((i%16==0)&&i!=0) printf("\n");
		}
	}
	printf("p\n");
	if(tempbuf_index >= buff_size){

		for(int i=0; i<buff_size; ++i){
			buff[i]=temporalbuffer[i];
		}

		lel=0;
		int err = snd_pcm_writei(handle,buff,frames);
		if(err<0){
			printf("error snd, prepare \n");
			snd_pcm_prepare(handle);
		}
		printf("sent! \n");
		for(int i=buff_size; i<temporalbuffer_size;++i){
			temporalbuffer[i-buff_size]=temporalbuffer[i];
		}
		tempbuf_index=0;
	}

	//transfer again
	if (libusb_submit_transfer(transfer) <0){
		//printf("Error re-submitting transfer \n");
		//return;
	}

}

int main(void)
{
	
	//PCM = pulse code modulation
	int err;
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
	buff_size = frames * channels *2; //el 2 es per el "sample size"
	
	temporalbuffer_size = (buff_size/ISOSIZE + ((buff_size%ISOSIZE)?1:0))*ISOSIZE;
	temporalbuffer = malloc(temporalbuffer_size);
	printf("temp buff size is %d \n",temporalbuffer_size);

	printf("buff size is %i \n" , buff_size);
	buff= malloc(buff_size);
	int tmp = 0;
	snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
	printf("tmp = : %d\n",tmp);


	int loops = (seconds*1000000)/tmp;
	printf("loops : %d\n",loops);

	//open device
	/*int fd = open("/dev/bus/usb/001/007", O_RDWR | O_NOCTTY);
	if(fd<0) {printf ("error opening %s \n", strerror(errno)); return -1;}
	set_interface_attribs(fd,B115200,0);
	close(fd);*/

	//goto label;

	libusb_device **devs = NULL;
	//libusb_device_handle *dev_handle;
	libusb_context *ctx = NULL;
	
	iniusblib(ctx);
	DeviceScan(ctx,devs);
	libusb_device_handle *dev_handle = selectDevice(ctx,2235,10690,2,3);

	struct libusb_transfer* trans;

	iniTransmission(dev_handle,trans);

	int r;	
	while(1){
		r = libusb_handle_events(NULL);
		if(r != LIBUSB_SUCCESS){
			printf("libusb handle events unsuccesfull \n");
			return 0;
		}
	}
	return 0;

	libusb_free_transfer(trans);
	libusb_free_device_list(devs,1);
	label:
	for (int l =0; l<=loops; ++l){
		//read
		err = read(0,buff,buff_size);
		if(err<0) printf("error de lectura. %s \n", strerror(errno));
		
		//echo(l,tmp,err,buff,buff_size,500,2);
		//synth(293,0,buff,buff_size);	
		//if(l<20)detectNote(buff,buff_size);
		//write
		err = snd_pcm_writei(handle,buff,frames);
		if(err<0) {
			printf("bon dia. %d\n", errno);
			snd_pcm_prepare(handle);
		}
		//if(err > 0 && err < (long)sizeof(buff)) printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buff),err);
	}
	return 0;
	r = libusb_release_interface(dev_handle, 0);
	if (r!=0){
		printf("cannot release interface\n");
	}
	printf("interface released");
	libusb_close(dev_handle);
	libusb_exit(ctx);
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
	double fd = (rate*2.0)/(double)f;
	//printf("period_samples = %lf \n", period_samples);
	for(int i=0; i<buff_size;++i){
		double value = (sin((inside_period*2.0*PI)/fd))*128 + 127;
		buff[i] = (char) value;
		++inside_period;
		if(inside_period>=fd)inside_period=0;
	}

	
	FILE* fid  = fopen("sortida2.txt", "w+");
	if(fid<=0)printf("lalala %s", strerror(errno));
	for(int i=0; i<buff_size;++i){
		fprintf(fid,"%d %d\n",i,buff[i]);
	}
	fclose(fid);

	
}

int detectNote( MYTYPE*buff, int buff_size){ //TODO TE UN GRAN PROBLEMA QUAN FD NO DIVIDEIX A BUFF_SIZE
	
	double fft_real[buff_size];
	double fft_img[buff_size];

	/*
	int f = 600;
	int fd = 2*rate / f;
	double dummy[buff_size];
	for(int i=0; i<buff_size;++i){
		//dummy[i] = sin(2*PI*i*1/fd);
		//printf("i : %d,  x : %lf \n",i,dummy[i]);	
	}*/
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

void DeviceScan(libusb_context *ctx, libusb_device **devs){
	
	size_t cnt = libusb_get_device_list(ctx, &devs); //get list of devices
	if (cnt<0){
		printf("error listing devices, %s \n", strerror(errno));
		exit(0);
	}
	for(size_t i=0; i<cnt;++i){
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(devs[i], &desc);
		if(r<0) {
			printf("error getting device info, %s \n",strerror(errno));
			exit(0);
		}
		struct libusb_config_descriptor *config;
		libusb_get_config_descriptor(devs[i],0,&config);
		printf("%d -> VendorID: %d,  ProductID %d, Ifaces %d\n", i,desc.idVendor, desc.idProduct, config->bNumInterfaces);
		const struct libusb_interface *inter;
		const struct libusb_interface_descriptor *interdesc;
		const struct libusb_endpoint_descriptor *epdesc;
		for (int i=0; i<config->bNumInterfaces; ++i){
			inter = &config->interface[i];
			printf ("    iz%d -> alternate settings: %d \n",i,inter->num_altsetting);
			for(int j=0; j<inter->num_altsetting;++j){
				interdesc = &inter->altsetting[j];
				printf("        Alt_set:%d, Endpoints: %d\n",j,interdesc->bNumEndpoints);
				for(int k=0; k<interdesc->bNumEndpoints; ++k){
					epdesc = &interdesc->endpoint[k];
					printf("            Ep:%d, DescriptorType: %d, Ep@: %d, MaxPacketSize %d \n",k,epdesc->bDescriptorType,epdesc->bEndpointAddress, epdesc->wMaxPacketSize);
				}
			}
			printf("\n");
		}
		printf("//////////////////\n");
	}
	
}

void iniusblib(libusb_context *ctx){
	int r = libusb_init(&ctx); //ini library
	if(r<0){
		printf("error ini library, %s \n",strerror(errno));
		exit(0);
	}
	libusb_set_debug(ctx,3); //verbose level 3
}

libusb_device_handle* selectDevice(libusb_context *ctx, int vid, int pid, int interface, int alt_setting){
	libusb_device_handle* dev_handle = libusb_open_device_with_vid_pid(ctx,vid,pid);

	printf("dev handle : %d\n",dev_handle);
	if(dev_handle==NULL){
		printf("cannot open device \n");
		exit(0);
	}else printf("device opened \n");

	if(libusb_kernel_driver_active(dev_handle, interface) == 1){
		printf("Kernel Driver Active \n");
		if(libusb_detach_kernel_driver(dev_handle,interface) ==0) printf("Kernel Driver Detached \n");
	}
	int r = libusb_claim_interface(dev_handle,interface);
	if(r<0){
		printf("couldn't claim interface %d \n",interface);
		exit(0);
	}
	r = libusb_set_interface_alt_setting(dev_handle,interface,alt_setting);
	if (r!=0){
		printf("couldnt set alt setting \n");
	}
	printf("interface %d claimed \n",interface);
	return dev_handle;
}

void iniTransmission(libusb_device_handle * dev_handle, struct libusb_transfer* trans){
	int received = 0;
	int size = 180;
	static uint8_t buffer[180]; //TODO: automatitzar max packet size
	for(int i=0; i<180; ++i) buffer[i]=0;
	int num_iso_pack = 1;
	trans = libusb_alloc_transfer(num_iso_pack);
	if (trans==NULL){
		printf("Error allocating transfer");
		exit(0);
	}
	libusb_fill_iso_transfer(trans,dev_handle,132,buffer,sizeof(buffer),num_iso_pack,callback,NULL,0);
	libusb_set_iso_packet_lengths(trans,sizeof(buffer)/num_iso_pack);
	//TODO: recomanen enviar mes d'un packet

	int r = libusb_submit_transfer(trans);
	printf("xxx\n");
	if(r!=0){
		printf("Error submiting transfer \n");
	}
}
