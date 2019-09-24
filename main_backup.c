#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
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

#define IP "192.168.1.155"
//#define IP "192.168.43.213"
#define SA struct sockaddr 
#define MYTYPE char

#define INTERFACE 2
#define ALTSETTING 1//1
#define ISOSIZE_I 196//196

#define INTERFACE_O 1
#define ALTSETTING_O 1//1 
#define ISOSIZE_O 192//192

char * savedbuff = NULL;
int savedbuff_index = 0;
int savedbuff_size = 0;

#define LOOP_N 4
#define LOOP_MAX 1048576 
char looping[LOOP_N][LOOP_MAX];
int looping_length[LOOP_N];
int looping_index[LOOP_N];


struct libusb_transfer* trans_o;
struct libusb_transfer* trans_i;

static char *device = "default";

void process_params(int argc, char* argv[]);
void DeviceScan(libusb_context *ctx, libusb_device **devs);
void iniusblib(libusb_context * ctx);
libusb_device_handle* selectDevice(libusb_context *ctx, int vid, int pid, int interface, int alt_setting);

void iniTransmission_i(libusb_device_handle * dev_handle, struct libusb_transfer* trans);
//void iniTransmission_o(libusb_device_handle * dev_handle, struct libusb_transfer* trans);
void iniTransmission_o(libusb_device_handle * dev_handle);
void th0_work();
void th1_work();
void th2_work();
void th3_work();

void applyeffects(char * ef_buff, int ef_buff_size);


//int detectNote( MYTYPE* buff, int buff_size);
char hola;

int buffindex=0;
int lmaoindex = 0;
char * lmao = (char*) -1;

int sent=0;
int received=0;

int pending = 0;

int fd_i;
int fd_o;
void callback_i(struct libusb_transfer* transfer){
	if (transfer->status != LIBUSB_TRANSFER_COMPLETED){
		printf("Transfer not completed. status = %d \n",transfer->status);
		return;
	}
	int realsize = 0;
	if (transfer->type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS){
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
	for(int i=0; i<realsize;i+=1){
		
		if((i%4)<2){//interleaved
			trans_o->buffer[buffindex] = transfer->buffer[i];
			savedbuff[savedbuff_index++] = transfer->buffer[i];
			if (savedbuff_index==savedbuff_size) savedbuff_index=0;
		}
		else trans_o->buffer[buffindex]=0;
	
		//read
	//	trans_o->buffer[buffindex] = bu[buffindex];
	
		++buffindex;
		if(buffindex==ISOSIZE_O){
			applyeffects(trans_o->buffer,ISOSIZE_O);
			buffindex = 0;
			//read
			//read(0,trans_o->buffer,ISOSIZE_O);	
			err = libusb_submit_transfer(trans_o);
			
			//write
			//for(int l=0;l<ISOSIZE_O;++l) write(fd_o,&trans_o->buffer[l],sizeof(trans_o->buffer[l]));
				//printf("%c",trans_o->buffer[l]);
			
			sent=1;
			if(err!=0) {
				printf("eeei? %d\n",err);
			}
		}
	}
	err = libusb_submit_transfer(transfer);
	if(err!=0) printf("eeei\n");
}

static void callback_o(struct libusb_transfer* transfer){
	sent = 0;
}


int nota; int playnota=0;
int detect_start=0,detect_finished=1;

struct effectparams{
	uint8_t enable,dist,echo,synt,eq,tremolo;
    	uint8_t in_v, out_v;
	uint8_t dist_mult,dist_ammount, dist_th, dist_soft, dist_hard;
    	uint8_t echo_speed, echo_volume;
	uint8_t eq_val;
	uint8_t tremolo_speed, tremolo_intensity;
};
struct effectparams presets[3];

struct effectparams * ep;

void applyeffects(char * ef_buff, int ef_buff_size){
	
	//printf("apply %d\n",ef_buff);
	//if(ep.enable) savebuff(buff,buff_size,savedbuff,savedbuff_size,&pos_in_savedbuff);
	//ep.in_v = 8;
	//highpass(ef_buff,ef_buff_size);
	//lowpass(ef_buff,ef_buff_size);

	//voleffect(ef_buff,ef_buff_size,0.001);
	//passeffect(ef_buff,ef_buff_size,0.01);

	if(ep.in_v!=8){
		double vol = ((double)(ep.in_v * ep.in_v))/64.0;
		buff_volume_adjust(ef_buff,0,ef_buff_size,vol);
	}
	if(ep.enable){
		/*if(ep.synt){
			if(detect_finished==1){
				detect_finished=0;
				detect_start=1;
				playnota = nota;
			}
			if(playnota>100) synth(playnota,0,buff,buff_size,rate);
			else bufftozero(buff,0,buff_size);
		}*/
		if(ep.echo) 
	
	  		echo(ef_buff, ef_buff_size, savedbuff, savedbuff_size, savedbuff_index, (double)((100-ep.echo_speed)*5.0)/200.0, (double)ep.echo_volume/100.0);


		//	echo(buff,buff_size,savedbuff,savedbuff_size,pos_in_savedbuff,
		  //   	     ep.echo_bpm,((double)ep.echo_when)/100.0,ep.echo_reps);
		//printf("%d\n",ep.dist_soft);
		if(ep.dist){
			newdist(ef_buff, ef_buff_size, ep.dist_mult, ep.dist_mult, ep.dist_th*280, 0.0, (double)ep.dist_ammount/100.0, ((double)(ep.dist_soft-4))/30.0);
			//lala(ef_buff,ef_buff_size,ep.dist_ammount/127.0,ep.dist_tone/256.0);
		
		//	lowpass(ef_buff,ef_buff_size,0.4);
		//	highpass(ef_buff,ef_buff_size,0.99);
		}
		if (ep.eq){
			lowhighpass(ef_buff, ef_buff_size, (double)ep.eq_val/100.0);
		}
		if (ep.tremolo){
			voleffect(ef_buff,ef_buff_size,(double)ep.tremolo_speed/100000.0,(double)ep.tremolo_intensity/100.0);
		}
	}

	//TODO: MOVE IT
/*	if (looping_length[0] < LOOP_MAX - 1){	
	for (int i=0; i<ef_buff_size && looping_length[0]<LOOP_MAX-1; i+=4){
		looping[0][looping_length[0]++]=ef_buff[i];		
		looping[0][looping_length[0]++]=ef_buff[i+1];		
	}//printf("recording\n");
	}else{
	//printf("playing\n");
	STYPE * sbuff = (STYPE *)&(ef_buff[0]);
	for (int i=0; i<ef_buff_size/BXS; i+=2){
		char low  = (looping[0][looping_index[0]++]);
		char high = (looping[0][looping_index[0]++]);
		STYPE loop = high;
		loop = loop<<8;
		loop += low&0x0FF;
		sbuff[i] += 0.6*loop;
		if (looping_index[0] == looping_length[0]) looping_index[0]=0;
	}
	}	
*/	

	if(ep.out_v!=8 && ep.enable){
		double vol = ((double)(ep.out_v * ep.out_v))/64.0;
		buff_volume_adjust(ef_buff,0,ef_buff_size,vol);
	}
	return;
}



int main(int argc, char* argv[])
{
	for(int i=0; i<LOOP_N; ++i){
		looping_index[i]=0;
		looping_length[i]=0;
	}	

	ep.dist_ammount=100;ep.dist_th=50;ep.dist_soft=5;ep.dist_hard=5;
	//ep.echo_bpm=60;ep.echo_when=100;ep.echo_reps=1;
	ep.echo_speed=50;ep.echo_volume=50;
	ep.eq_val=50;
	ep.tremolo_speed=50;ep.tremolo_intensity=50;
	ep.in_v=8;ep.out_v=8;
	ep.dist=0;ep.echo=0;ep.synt=0;ep.eq=0;ep.tremolo=0;
	ep.enable = 0;
	//ep = {0,100,100,1,8,8,0,0,0};
	savedbuff_size = (BXS*MAXSECONDS*RATE)/INTERLEAVED;
	printf("savedbuff_size: %d\n",savedbuff_size);	
	savedbuff = malloc(savedbuff_size);

	fd_i=open("input.txt", O_RDWR | O_CREAT);
	fd_o=open("output.txt", O_RDWR | O_CREAT);
	
	pthread_t socketthread,socketthread1;
	if (pthread_create(&socketthread,NULL,(void *)th1_work,0)){
		printf("error creating thread\n");
	}
/*	if (pthread_create(&socketthread1,NULL,(void *)th2_work,0)){
		printf("error creating thread\n");
	}
*/	if (pthread_create(&socketthread1,NULL,(void *)th3_work,0)){
		printf("error creating thread\n");
	}
	inisintable();
	inicostable();
	//iniDiode(450, 4, 7);

/*
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
	*/
	th0_work();
	return 0;
}

void th0_work(){

	//libusb
	libusb_device **devs = NULL;
	libusb_context *ctx_i = NULL;
	
	iniusblib(ctx_i);
	
	DeviceScan(ctx_i,devs);
 	libusb_device_handle *dev_handle_i = selectDevice(ctx_i,2235,10690,INTERFACE,ALTSETTING);
	iniTransmission_i(dev_handle_i,trans_i);
	int r = 0;
	while(1){
  		r = libusb_handle_events(ctx_i);
  		if(r != LIBUSB_SUCCESS){
  			printf("libusb handle events unsuccesfull \n");
  		}
        }
  	libusb_free_transfer(trans_i);
        libusb_free_device_list(devs,1);
  	r = libusb_release_interface(dev_handle_i, 0);
  	if (r!=0)printf("cannot release interface\n");
	printf("interface released");
  	libusb_close(dev_handle_i);
	libusb_exit(ctx_i);
}
void th3_work(){

	libusb_device **devs = NULL;
	libusb_context *ctx_o = NULL;
	iniusblib(ctx_o);
	//DeviceScan(ctx_i,devs);
  	libusb_device_handle *dev_handle_o = selectDevice(ctx_o,2235,10690,INTERFACE_O,ALTSETTING_O);
	//transfers are global variables now 
	//struct libusb_transfer* trans_i;
	iniTransmission_o(dev_handle_o); //la output es global
	int r = 0;
	while(1){
  		r = libusb_handle_events(ctx_o);
  		if(r != LIBUSB_SUCCESS){
  			printf("libusb handle events unsuccesfull \n");
  		}
        }
        libusb_free_device_list(devs,1);
	r = libusb_release_interface(dev_handle_o, 0);
	if (r!=0)printf("cannot release interface\n");
	printf("interface released");
  	libusb_close(dev_handle_o);
	libusb_exit(ctx_o);
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
    	if(inet_pton(AF_INET,IP, &servaddr.sin_addr)<=0) printf("ERROR addr not suported");
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
    	


	/*trames de 4 Bytes
	
	General: E=Enable
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|0000, EXXX	|    XXXXXXXX   |   XXXXXXXX    |     XXXXXXX	|
	+-------+-------+-------+-------+-------+-------+-------+-------+
	
	volume:
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|0001, xxxx	| IIII  , OOOO  |   XXXXXXXX    |     XXXXXXX	|
	+-------+-------+-------+-------+-------+-------+-------+-------+

	dist:
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|0010, Ennn	|    dddddddd   |   tttttttt    |  ssss , hhhh 	|
	+-------+-------+-------+-------+-------+-------+-------+-------+
	
	echo:
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|0011, Errr	|    bbbbbbbb   |   wwwwwwww    |   XXXXXXXX	|
	+-------+-------+-------+-------+-------+-------+-------+-------+

	syn:
	+-------+-------+-------+-------+-------+-------+-------+-------+
	|0100, EXXX	|    XXXXXXXX   |   XXXXXXXX    |   XXXXXXXX	|
	+-------+-------+-------+-------+-------+-------+-------+-------+

	*/
	int s = 4;
    	unsigned char buff[4];
	uint16_t th,soft,hard;
	uint8_t nmessages = 0;
    	while(1){
    		read(connfd,&nmessages,1);
		printf("nmessages = %d\n",nmessages);
		for(int nm=0; nm<nmessages; ++nm){

		for(int i=0; i<s;++i)buff[i]=0;
    		read(connfd,buff,sizeof(buff));
		//printf("code op: %02x\n",buff[s-1]);
		switch(buff[s-1]>>4){
			case CODE_GEN:
				ep.enable = (char)buff[s-1]&0x08;
				//ep.enable=1;
				printf("gen: enable=%d\n",ep.enable);
				break;
			case CODE_VOL:
				ep.in_v  = buff[s-2]>>4;
				ep.out_v = buff[s-2]&0x0F;
				printf("vol: in=%d out=%d\n",ep.in_v,ep.out_v);
				break;
			case CODE_DIS:
				ep.dist = (char)buff[s-1]&0x08;
				ep.dist_mult = buff[s-1]&0x07;
				ep.dist_ammount = buff[s-2];
				th = buff[s-3];
				soft = buff[s-4]>>4;
				hard = buff[s-4]&0x0F;
				if(th!=ep.dist_th || soft!=ep.dist_soft || hard!=ep.dist_hard){
					//iniDiode((int)th<<1, soft, hard);
					//fin1_p, multini, linfin, softfin, hardfin){
					iniDiode2((double)th/2048.0,1.0,0.95,(double)soft/10.0,11-hard);
				}
				ep.dist_th = th;
				ep.dist_soft = soft;
				ep.dist_hard = hard;
				printf("dis: enable=%d mult=%d amm=%d th=%d s=%d h=%d\n",ep.dist, ep.dist_mult, ep.dist_ammount,ep.dist_th,ep.dist_soft,ep.dist_hard);
				break;
			case CODE_ECH:
				ep.echo = (char)buff[s-1]&0x08;
				ep.echo_speed = buff[s-2];
				ep.echo_volume = buff[s-3];
				printf("echo: enable=%d speed=%d volume=%d when=%d\n",ep.echo,ep.echo_speed,ep.echo_volume);
				break;
			case CODE_SYN:
				ep.synt = (char)buff[s-1]&0x08;
				printf("syn: enable=%d\n",ep.synt);
				break;
			case CODE_EQ:
				ep.eq = (char)buff[s-1]&0x08;
				ep.eq_val = buff[s-2];
				printf("eq: enable=%d val=%d \n",ep.eq,ep.eq_val);
				break;
			case CODE_TREM:
				ep.tremolo = (char)buff[s-1]&0x08;
				ep.tremolo_speed = buff[s-2];
				ep.tremolo_intensity = buff[s-3];
				printf("tremolo: enable=%d speed=%d intensity=%d \n",ep.tremolo,ep.tremolo_speed, ep.tremolo_intensity);
				break;
			default:
				break;
				
		}//switch		
		}//nmessages
		printf("correctly read %d messages\n",nmessages);
		//ep.enable = 1;
		connfd = accept(sockfd, (SA*)&cli, &len); 
    } 
    //close the socket 
    close(sockfd); 
}
void th2_work(){
	while(1);
/*	while(1){
		while(!detect_start);
		detect_start = 0;
		detect_finished = 0;
		nota = detectNote(savedbuff,savedbuff_size,pos_in_savedbuff,rate);
		detect_finished = 1;
	}
*/
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
		if(libusb_detach_kernel_driver(dev_handle,interface) ==0) 
			printf("Kernel Driver Detached \n");
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
	printf("interface %d claimed, alt setting %d\n",interface, alt_setting);
	return dev_handle;
}


void iniTransmission_i(libusb_device_handle * dev_handle, struct libusb_transfer* trans){
	int received = 0;
	static uint8_t tramabus[ISOSIZE_I]; 
	for(int i=0; i<ISOSIZE_I; ++i) tramabus[i]=0;
	int num_iso_pack = 1;
	trans_i = libusb_alloc_transfer(num_iso_pack);
	if (trans_i==NULL){
		printf("Error allocating transfer");
		exit(0);
	}
	libusb_fill_iso_transfer(trans_i,dev_handle,132,tramabus,sizeof(tramabus),num_iso_pack,callback_i,NULL,0);
	libusb_set_iso_packet_lengths(trans_i,sizeof(tramabus)/num_iso_pack);
	//TODO: recomanen enviar mes d'un packet
	int r = libusb_submit_transfer(trans_i);
	if(r!=0)printf("Error submiting transfer \n");
}

void iniTransmission_o(libusb_device_handle * dev_handle/*, struct libusb_transfer* trans*/){
	int received = 0;
	static uint8_t tramabus[ISOSIZE_O]; 
	for(int i=0; i<ISOSIZE_O; ++i) tramabus[i]=0;
	int num_iso_pack = 1;
	//trans = libusb_alloc_transfer(num_iso_pack);
	trans_o = libusb_alloc_transfer(num_iso_pack);
	if (trans_o==NULL){
		printf("Error allocating transfer");
		exit(0);
	}
	libusb_fill_iso_transfer(trans_o,dev_handle,2,tramabus,sizeof(tramabus),num_iso_pack,callback_o,NULL,0);
	libusb_set_iso_packet_lengths(trans_o,sizeof(tramabus)/num_iso_pack);
	//TODO: recomanen enviar mes d'un packet
//	int r = libusb_submit_transfer(trans);
//	if(r!=0)printf("Error submiting transfer \n");
}
