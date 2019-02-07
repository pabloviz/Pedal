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

char volume_adjust(char c, double volume){
	double dc = (double)c;
	if(c>=127){
		if(volume>50){ 
			return (char)((volume*(5.1-dc/50.0)-255.0+2.0*dc)); //potser amb % seria mes facil
		}else{
			return (char)(volume*(dc/50.0 - 2.54)+127);
		}
	}
	else{
		if(volume>50){
			return (char)(((-dc*volume)/50.0) + 2.0*dc);
		}else{
			return (char)(volume*(dc/50 - 2.54)+127);
		}
	}

}


/*
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
		//soc un geni de les caches.
		for(int j=1; j<=layers; ++j){
			int index = tmp_howmany*j*buff_size;//quantes mostres enrere
			//index-= (buff_size*1 +1);
			--index;
			//index = 80000;
			printf("index : %d\n",index);	
			for(int i=0; i<buff_size; ++i){
				char new = buff[i];
				if (0){//no ens cal echobuff
					//buff[i] = buff[i] + buff[i-index];

				}else{ //ens cal anar al echobuff
					int tmp = index/buff_size + (index!=buff_size?1:0);
					if(tmp<=echobuff_index) tmp = echobuff_index - tmp;
					else tmp = howmany+(echobuff_index-tmp);
					char old = echobuff[tmp*buff_size + i];
					buff[i] = buff[i] + old;	
				}
				if(j==1) echobuff[(echobuff_index)*buff_size + i] = new;
			}
		}
		++echobuff_index;
		if(echobuff_index >= howmany)echobuff_index=0;

}*/
