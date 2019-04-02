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
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h> 
//#include <netdb.h> 
//#include <netinet/in.h> 
#include <arpa/inet.h> //el pton
#include "notes.h"
#include "effects.h"
#include "utils.h"
#include "gpioControl.h"
#include "Diode.h"

#define SA struct sockaddr 
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
char* savedbuff;
int pos_in_savedbuff;
int period;
//char *tramabus;
int buff_size;
int savedbuff_size;
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


int buffindex=0;
int pablo = 0;
double ttime = 0;
int lalal =0;
int lmaoindex = 0;
char * lmao = (char*) -1;
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
	if(pablo<10000)++pablo;
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
	
	int read_v = 0;
	int write_v = 0;

	int err;
	unsigned char bu[realsize]; 
	if (lmao == (char*)-1 && write_v==1) {
		lmao = (char *)malloc(500000);
	}
	if(read_v)read(0,bu,realsize);
		
	for(int i=0; i<realsize;i+=1){
		
		buff[buffindex] = transfer->buffer[i];
		if(read_v)buff[buffindex] = bu[i];
		++buffindex;
		if(write_v) {
			if(lmaoindex==500000){
				//buff_volume_adjust(lmao,0,lmaoindex,4);
				lala(lmao,lmaoindex,2,1);
				for(int i=0; i<lmaoindex; ++i)
					printf("%c",lmao[i]);
				exit(0);
			}
			if(!read_v)lmao[lmaoindex] = transfer->buffer[i];
			else lmao[lmaoindex] = bu[i];
			//printf("%c",buff[buffindex]);
			++lmaoindex;
		}

		if(buffindex>=buff_size){
			//printf(" HOLA \n");
		 	//if(write_v)printbuff(buff,buff_size);
			//for(int i=0; i<1024; ++i) printf("%c",buff[i]);
			//exit(0);
			//printf("ini\n");

	
			//lala(buff,buff_size,2,1);
			savebuff(buff,buff_size,savedbuff,savedbuff_size,&pos_in_savedbuff);
			applyeffects();
			

			//printf("Pablo %d\n",pablo);
			/*if(pablo>=6*12){
				for(int j=0; j<buff_size*12; ++j) printf("%c",savedbuff[j]);
				exit(0);
			}*/	
	
			//applyeffects();

			//if(pablo>=2000){
				//for(int i=0; i<1024; ++i) printf("%c",buff[i]);
				//exit(0);
			//}
			err = snd_pcm_writei(handle,buff,frames);
			if(err<0){
				printf("error snd, prepare , %s\n",strerror(errno));
				snd_pcm_prepare(handle);
			}
			buffindex=0;
			//printf("fin\n");
			//exit(0);
			if(read_v && !write_v && pablo>=2500) exit(0);
		}
	}
	err = libusb_submit_transfer(transfer);
	if(err!=0) printf("eeei\n");
}

int nota; int playnota=0;
int detect_start=0,detect_finished=1;

struct effectparams{
    	int dist_ammount,echo_bpm,echo_when,echo_reps;
    	short in_v,out_v,dist_type;
    	char dist,echo,synt;
};
struct effectparams ep;

void applyeffects(){
	//ep.dist_ammount = 100;
	//printf(" %d %lf \n",ep.in_v, ((double)ep.in_v*(double)ep.in_v)/64.0);
//	ep.dist=1;
	//ep.dist_ammount=64;
	//ep.synt = 1;
	//
	//synth(440,0,buff,buff_size,rate);
	//lala(buff,buff_size,ep.dist_ammount/128,1);

	if(ep.in_v!=8){
		double vol = ((double)(ep.in_v * ep.in_v))/64.0;
		buff_volume_adjust(buff,0,buff_size,vol);
	}
	if(ep.synt){
		if(detect_finished==1){
			detect_finished=0;
			detect_start=1;
		//	detect_finished=0;
			playnota = nota;
			//printf("%d\n",playnota);
		}
		if(playnota>100) synth(playnota,0,buff,buff_size,rate);
		else bufftozero(buff,0,buff_size);
	}
	/*if((pos_in_savedbuff%2048)==0){
		int ndn = detectNote(buff,buff_size,savedbuff,savedbuff_size,pos_in_savedbuff,rate);
		printf("note : %d\n",ndn);
	}*/
	if(ep.echo) 
		echo(buff,buff_size,savedbuff,savedbuff_size,pos_in_savedbuff,
		     ep.echo_bpm,((double)ep.echo_when)/100.0,ep.echo_reps);
	//return;
	if(ep.dist)
		//distorsion(buff,buff_size,ep.dist_ammount,ep.dist_type);
		lala(buff,buff_size,ep.dist_ammount/128.0,ep.dist_type*ep.dist_type);

	if(ep.out_v!=8){
		double vol = ((double)(ep.out_v * ep.out_v))/64.0;
		buff_volume_adjust(buff,0,buff_size,vol);
	}
	return;
	/*
	if(mask&0x01){
		if(th1_valid==1){
			playnota=nota*(playnota!=-10);
			th1_valid=0;
		}
		if(th1_ready==1){
			//for(int i=0; i<buff_size; ++i) th1buff[i]=buff[i];
			th1_start=1;
		}
		if(playnota>100)synth(playnota,0,buff,buff_size,rate);
		else bufftozero(buff,0,buff_size);
	}	
	if(mask&0x02){
		//echo(period,buff,buff_size,echo_time,echo_reps);
	}
	if(mask&0x04){
		distorsion(buff,buff_size,dist_ammount,dist_type);
	}
	
	
	if(out_v!=1)buff_volume_adjust(buff,0,buff_size,out_v);*/

}



int main(int argc, char* argv[])
{
	ep.dist_ammount=0;ep.echo_bpm=60;ep.echo_when=100;ep.echo_reps=1;
	ep.in_v=8;ep.out_v=8;ep.dist_type=2;ep.dist=0;ep.echo=0;
	//ep = {0,100,100,1,8,8,0,0,0};
	pthread_t socketthread,socketthread1;
	if (pthread_create(&socketthread,NULL,(void *)th1_work,0)){
		printf("error creating thread\n");
	}
	if (pthread_create(&socketthread1,NULL,(void *)th2_work,0)){
		printf("error creating thread\n");
	}
	//process_params(argc,argv);
	//dist_ammount=18;echo_time=1000;echo_reps=4;nthreads=3;
	inisintable();
	//printf("%lf\n",getSin(1.5*PI));
	inicostable();
	//th[0-450-100] soft[0-4-10] hard [0-7-10]
	iniDiode(5000, 9, 7);
	//iniDiodeTable();
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
	int r;
	nthreads=1;
	//goto th0;
	


	th0_work();
	/*
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
	}*/
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
	savedbuff_size = (1000*SAVED_mSECONDS)/period;
	savedbuff_size *= buff_size;
	savedbuff_size;
	//savedbuff_size /= 2; //INTERLEAVED
	printf("savedbuff_size = %d\n",savedbuff_size);
	savedbuff = (char *)malloc(savedbuff_size);
	for(int i=0; i<savedbuff_size;++i)savedbuff[i]=0;
	pos_in_savedbuff = 0;
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
    
	int sockfd, connfd, len; 
	struct sockaddr_in servaddr, cli; 
	int socket_port = 8080;
    	//socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    	if (sockfd == -1) { 
        	printf("socket creation failed...\n"); 
        	exit(0); 
    	}else printf("Socket successfully created..\n"); 
    	bzero(&servaddr, sizeof(servaddr));
    	// assign IP, PORT 
    	servaddr.sin_family = AF_INET; 
    	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    	servaddr.sin_port = htons(socket_port); 
    	if(inet_pton(AF_INET,"192.168.1.155", &servaddr.sin_addr)<=0) printf("ERROR addr not suported");
    	// Binding newly created socket to given IP and verification 
    	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        	printf("socket bind failed...\n"); 
        	exit(0); 
    	}else printf("Socket successfully binded..\n"); 
    	// Now server is ready to listen and verification 
    	if ((listen(sockfd, 5)) != 0) { 
        	printf("Listen failed...\n"); 
        	exit(0); 
    	}else printf("Server listening..\n"); 
   	 len = sizeof(cli); 
    	// Accept the data packet from client and verification 
    	connfd = accept(sockfd, (SA*)&cli, &len); 
    	if (connfd < 0) { 
       	 	printf("server acccept failed...\n"); 
        	exit(0); 
    	}else printf("server acccept the client...\n");
    	int s = 6;
    	unsigned char buff[s];

    	while(1){
    		for(int i=0; i<s;++i)buff[i]=0;
    		read(connfd,buff,sizeof(buff));
		ep.in_v = (short)(buff[s-1]>>4);
		ep.out_v = (short)(buff[s-1]&0x0F);
		ep.synt = (char)((buff[s-2]>>4)&0x01);
		ep.dist = (char)((buff[s-2]>>3)&0x01);
		ep.dist_type = (short)(buff[s-2]&0x07);
		ep.dist_ammount = (int)(((buff[s-3]>>1))&0x7F);
		ep.echo = (char)(buff[s-3]&0x01);
		ep.echo_bpm = (int)buff[s-4];
		ep.echo_when = (int)buff[s-5];
		ep.echo_reps = (int)buff[s-6];
		printf("client sended->\n in_v:%d out_v:%d \n synt:%d \ndist:%d dist_t:%d dist_a:%d\n echo:%d echo_bpm:%d echo_when:%d echo_reps:%d\n",ep.in_v,ep.out_v,ep.synt,ep.dist,ep.dist_type,ep.dist_ammount,ep.echo,ep.echo_bpm,ep.echo_when,ep.echo_reps);
		connfd = accept(sockfd, (SA*)&cli, &len); 
    } 
    //close the socket 
    close(sockfd); 
}
void th2_work(){

	while(1){
		while(!detect_start);
		detect_start = 0;
		detect_finished = 0;
		nota = detectNote(savedbuff,savedbuff_size,pos_in_savedbuff,rate);
		detect_finished = 1;
	}

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
	if(err<0) printf("Cant set interleaved mode. %s \n", strerror(errno));

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
	//th1buff = malloc(buff_size);
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
/*	
	if(argc<4) printf("falten params:\n sudo ./main.o in_v out_v mask dist_type/echo_time dist_ammount/echo_reps\n");
	ep.in_v = atoi(argv[1]);
	ep.out_v= atoi(argv[2]);
	mask = atoi(argv[3]);
	if((mask&0x01)!=0) nthreads=3;
	else nthreads=2;

	if((mask&0x04)!=0){
		ep.dist_type   =atoi(argv[4]);
		ep.dist_ammount=atoi(argv[5]);	
	}
	if((mask&0x02)!=0){
		ep.echo_time = atoi(argv[4]);
		ep.echo_reps = atoi(argv[5]);
	}*/
}
