#include "gpioControl.h"


int gpio_ini(){

	if((mem_fd = open("/dev/mem", O_RDWR|O_SYNC))<0){
		printf("can't open /dev/mem \n");
		return -1;
	}
	gpio_map = mmap(NULL,BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE);
	close(mem_fd);
	if(gpio_map == MAP_FAILED){
		printf("mmap error %d\n", (int)gpio_map);
		return -1;
	}
	gpio = (volatile unsigned *) gpio_map;
	//*(gpio)=0;
	return 0;
	
}
void gpio_setInput(int pin_num){

	*(gpio+(pin_num/10)) &= ~(7<<((pin_num%10)*3));
}
void gpio_setOutput(int pin_num){

	*(gpio+(pin_num/10)) &= ~(7<<((pin_num%10)*3));
	*(gpio+(pin_num/10)) |=  (1<<((pin_num%10)*3));
	gpio_setValue(pin_num,0);
}

void gpio_setValue(int pin_num, int value){
	
	if(value) *(gpio+7) = 1 << pin_num;
	else *(gpio+10) = 1 << pin_num;


	//*(gpio+7) = (value << pin_num);
/*	
	if(value) *(gpio+7) |= (value << pin_num);
	else *(gpio+7) = (*(gpio+13)&(0xFFFFFFFF^(1<<pin_num)));*/
}

int gpio_readValue(int pin_num){
	//return (((*(gpio+7))>>pin_num)&0x01);
	return *(gpio+13)&=1<<pin_num;
}


