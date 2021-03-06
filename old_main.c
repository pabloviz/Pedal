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
#include "effects.h"
#include "fft.h"
#include "utils.h"
#include "gpioControl.h"

#define MYTYPE char
#define ISOSIZE 180
#define INTERFACE 2
#define ALTSETTING 3

static char *device = "default";

snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
int rate = 44100;
int channels = 2;
int seconds = 5;
snd_pcm_sframes_t frames;
char* buff;
char* th1buff;
int period;
//char *tramabus;
int buff_size;
int mask = 0;
int nthreads=0;

void process_params(int argc, char* argv[]);
void DeviceScan(libusb_context *ctx, libusb_device **devs);
void init_output();
void iniusblib(libusb_context * ctx);
libusb_device_handle* selectDevice(libusb_context *ctx, int vid, int pid, int interface, int alt_setting);
void iniTransmission(libusb_device_handle * dev_handle, struct libusb_transfer* trans);

void th0_work();
void th1_work();
void th2_work();

/*
int howmany=-1;
MYTYPE * echobuff = -1;
void echo(int loop, int period,int read, MYTYPE* buff, int buff_size, int ret, int layers);//layers default = 1
void synth(int f, int instr, MYTYPE* buff, int buff_size);
*/void applyeffects();


//int detectNote( MYTYPE* buff, int buff_size);
char hola;


void trap(){
	while(1);
}


int buffindex=0;
int pablo = 0;
double ttime = 0;
int lalal =0;
static void callback(struct libusb_transfer* transfer){

	//struct timeval tv1;
	//gettimeofday(&tv1,NULL);
	//printf("time: %d\n",(tv1.tv_usec-lalal));
	//lalal = tv1.tv_usec;


	//double tmp = clock();
	//printf("time: %lf\n", tmp-ttime);
	//ttime=clock();
	
	
	//double tmp = omp_get_wtime();
	//printf("time: %hf\n", tmp-ttime);
	//ttime=tmp;


	//printf("thread num %d\n",omp_get_thread_num());
	++pablo;
	if (transfer->status != LIBUSB_TRANSFER_COMPLETED){
		printf("Transfer not completed. status = %d \n",transfer->status);
		return;
	}
	int realsize = 0;
	if (transfer->type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS){ //les iso pden tenir errors puntuals
		for(int i=0; i<transfer->num_iso_packets; ++i){
			struct libusb_iso_packet_descriptor *packet = &transfer->iso_packet_desc[i];
			if (packet->status != LIBUSB_TRANSFER_COMPLETED){
				printf("Error in packet number %d of %d \n",i,transfer->num_iso_packets);
				return;
			}
			realsize = packet->actual_length;
		}
	}
	int err;
	//char bu[realsize];	
	//read(0,bu,realsize);	
	for(int i=0; i<realsize;i+=1){ //TODO: per accelerar..en realitat aquest buffer jo el tinc
		buff[buffindex] = transfer->buffer[i];
		//buff[buffindex] = bu[i];
		++buffindex;
		if(buffindex>=buff_size){
			//applyeffects();
			err = snd_pcm_writei(handle,buff,frames);
			if(err<0){
				printf("error snd, prepare , %s\n",strerror(errno));
				snd_pcm_prepare(handle);
			}
			buffindex=0;
		}
	}
	err = libusb_submit_transfer(transfer);
	if(err!=0) printf("eeei\n");
}
int th1_start=0; int th1_valid=0; int th1_ready=1;
int nota; int playnota=0;
int in_v=1; int out_v=1;
int dist_type=0; int dist_ammount=0;
int echo_time=0; int echo_reps=0;
void applyeffects(){

	printf("xd\n");
	//printbuff(buff,buff_size);
	return;
	//mask = 4;
	
	if(in_v!=1)buff_volume_adjust(buff,0,buff_size,in_v);	

	if(mask&0x01){
		if(th1_valid==1){
			playnota=nota*(playnota!=-10);
			th1_valid=0;
		}
		if(th1_ready==1){
			for(int i=0; i<buff_size; ++i) th1buff[i]=buff[i];
			th1_start=1;
		}
		if(playnota>100)synth(playnota,0,buff,buff_size,rate);
		else bufftozero(buff,0,buff_size);
	}	
	if(mask&0x02){
		echo(period,buff,buff_size,echo_time,echo_reps);
	}
	if(mask&0x04){
		distorsion(buff,buff_size,dist_ammount,dist_type);
	}
	
	
	if(out_v!=1)buff_volume_adjust(buff,0,buff_size,out_v);

}



int main(int argc, char* argv[])
{
	//process_params(argc,argv);
	dist_ammount=18;echo_time=1000;echo_reps=4;nthreads=3;
	inisintable();
	//printf("%lf\n",getSin(1.5*PI));
	inicostable();
	//printf("%lf\n",getCos(100));



	int err;
	err = gpio_ini();
	if(err<0){
		printf("error in gpio_ini\n");
		exit(EXIT_FAILURE);
	}
	gpio_setOutput(OUT_DIST);
	gpio_setOutput(OUT_ECHO);
	gpio_setOutput(OUT_SYNT);

	gpio_setInput(BTN_DIST);
	gpio_setInput(BTN_ECHO);
	gpio_setInput(BTN_SYNT);

	gpio_setInput(BTN_ECHO);
	gpio_setInput(BTN_SYNT);
	gpio_setInput(BTN_VLUP);
	gpio_setInput(BTN_VLUP);
	gpio_setInput(BTN_VLDW);
	//init_output();
/*
	//PCM = pulse code modulation
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
	
	//temporalbuffer_size = (buff_size/ISOSIZE + ((buff_size%ISOSIZE)?1:0))*ISOSIZE;
	//temporalbuffer = malloc(temporalbuffer_size);
	//printf("temp buff size is %d \n",temporalbuffer_size);

	printf("buff size is %i \n" , buff_size);
	buff= malloc(buff_size);
	th1buff = malloc(buff_size);
	//int tmp = 0;
	snd_pcm_hw_params_get_period_time(params, &period, NULL);
	printf("tmp = : %d\n",period);


	int loops = (seconds*1000000)/period;
	printf("loops : %d\n",loops);
*/
	//usb
	/*

	libusb_device **devs = NULL;
	//libusb_device_handle *dev_handle;
	libusb_context *ctx = NULL;
	
	iniusblib(ctx);
	//DeviceScan(ctx,devs);
	libusb_device_handle *dev_handle = selectDevice(ctx,2235,10690,INTERFACE,ALTSETTING);

	struct libusb_transfer* trans;

	iniTransmission(dev_handle,trans);
*/
	int r;
	nthreads=2;
	//goto th0;
	omp_set_num_threads(nthreads);
	#pragma omp parallel
	{
	printf("soc %d\n",omp_get_thread_num());
	if(omp_get_thread_num()==0) th0_work();
	else if(omp_get_thread_num()==1) th1_work();
	else if(omp_get_thread_num()==2) th2_work();
	else{
		printf("unexpected thread. closing \n");
		exit(0);
	}
	}
	//return 0;
	/*
	libusb_free_transfer(trans);
	libusb_free_device_list(devs,1);
	r = libusb_release_interface(dev_handle, 0);
	if (r!=0){
		printf("cannot release interface\n");
	}
	printf("interface released");
	libusb_close(dev_handle);
	libusb_exit(ctx);*/
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	
	free(buff);
	free(echobuff);
	return 0;
}

void th0_work(){

	init_output();	
	//libusb
	libusb_device **devs = NULL;
	libusb_context *ctx = NULL;	
	iniusblib(ctx);
	//DeviceScan(ctx,devs);
	libusb_device_handle *dev_handle = selectDevice(ctx,2235,10690,INTERFACE,ALTSETTING);
	struct libusb_transfer* trans;
	iniTransmission(dev_handle,trans);

	while(1){
		//printf("time: %d\n", clock()-ttime);
	//	ttime=clock();
		int r = libusb_handle_events(ctx);
		if(r != LIBUSB_SUCCESS){
			printf("libusb handle events unsuccesfull \n");
			//return 0;
		}
	}
	//libusb
	libusb_free_transfer(trans);
	libusb_free_device_list(devs,1);
	int r = libusb_release_interface(dev_handle, 0);
	if (r!=0)printf("cannot release interface\n");
	printf("interface released");
	libusb_close(dev_handle);
	libusb_exit(ctx);
}
void th1_work(){
	while(1){
		//printf("time: %d\n", clock()-ttime);
		//ttime=clock();
		/*	
		//printf("x\n");	
		if(gpio_readValue(BTN_DIST)) {
			mask ^= 1<<2;
			gpio_setValue(OUT_DIST,mask&(1<<2));
			usleep(500000);
		}
		if(gpio_readValue(BTN_ECHO)) {
			mask ^= 1<<1;
			gpio_setValue(OUT_ECHO,mask&(1<<1));
			usleep(500000);
		}
		if(gpio_readValue(BTN_SYNT)) {
			mask ^= 1<<0;
			gpio_setValue(OUT_SYNT,mask&(1<<0));
			usleep(500000);
		}
		if(gpio_readValue(BTN_VLUP)){
			printf("%d\n",in_v);
			++in_v;
			usleep(500000);
		}
		if(gpio_readValue(BTN_VLDW)){
			if(in_v>0)--in_v;
			printf("%d\n",in_v);
			usleep(500000);
		}*/
	}
}
void th2_work(){
		//++pablo;
		/*
		while(!th1_start);
		th1_ready=0;
		th1_start=0;
		nota = detectNote(th1buff,buff_size,rate);
		//printf("nota: %d\n", nota);
		if(nota>100 || nota==-10){
			th1_valid=1;
			//printf("nota: %d\n",nota);
		}
		while(th1_valid==1);//espera a la consumicio
		th1_ready=1; //es preparar per rebre noves dades
		//else th1_valid=0;
		*/

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

void init_output(){

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
	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	buff_size = frames * channels *2; //el 2 es per el "sample size"
	
	//temporalbuffer_size = (buff_size/ISOSIZE + ((buff_size%ISOSIZE)?1:0))*ISOSIZE;
	//temporalbuffer = malloc(temporalbuffer_size);
	//printf("temp buff size is %d \n",temporalbuffer_size);

	printf("buff size is %i \n" , buff_size);
	buff= malloc(buff_size);
	th1buff = malloc(buff_size);
	//int tmp = 0;
	snd_pcm_hw_params_get_period_time(params, &period, NULL);
	printf("tmp = : %d\n",period);


	int loops = (seconds*1000000)/period;
	printf("loops : %d\n",loops);
}


void iniTransmission(libusb_device_handle * dev_handle, struct libusb_transfer* trans){
	int received = 0;
	//int size = 180;
	//static uint8_t buffer[180]; //TODO: automatitzar max packet size
	//tramabus = malloc(ISOSIZE);
	static uint8_t tramabus[ISOSIZE];

	for(int i=0; i<ISOSIZE; ++i) tramabus[i]=0;
	int num_iso_pack = 1;
	trans = libusb_alloc_transfer(num_iso_pack);
	if (trans==NULL){
		printf("Error allocating transfer");
		exit(0);
	}
	libusb_fill_iso_transfer(trans,dev_handle,132,tramabus,sizeof(tramabus),num_iso_pack,callback,NULL,0);
	libusb_set_iso_packet_lengths(trans,sizeof(tramabus)/num_iso_pack);
	//TODO: recomanen enviar mes d'un packet

	int r = libusb_submit_transfer(trans);
	if(r!=0){
		printf("Error submiting transfer \n");
	}
}
void process_params(int argc, char* argv[]){
	
	if(argc<4) printf("falten params:\n sudo ./main.o in_v out_v mask dist_type/echo_time dist_ammount/echo_reps\n");
	in_v = atoi(argv[1]);
	out_v= atoi(argv[2]);
	mask = atoi(argv[3]);
	if((mask&0x01)!=0) nthreads=3;
	else nthreads=2;

	if((mask&0x04)!=0){
		dist_type   =atoi(argv[4]);
		dist_ammount=atoi(argv[5]);	
	}
	if((mask&0x02)!=0){
		echo_time = atoi(argv[4]);
		echo_reps = atoi(argv[5]);
	}
}
