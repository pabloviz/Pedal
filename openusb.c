#include "openusb.h"


int set_interface_attribs (int fd, int speed, int parity){
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if(tcgetattr (fd, &tty) != 0){
		printf("error %d from tcgeatttr : ", errno);
		//return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_iflag &= ~IGNBRK;
	tty.c_oflag = 0;
	tty.c_cc[VMIN] = 0; //no block
	tty.c_cc[VTIME] = 5; //0.5 s timeout
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr(fd,TCSANOW,&tty) !=0){
		printf("error %d from tcsetattr", errno);
	}
}

/*
char *portname = "/dev/bus/usb/001/007"; //TODO : puc automatitzar aixo 

int main(void){

	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	if (fd<0){
		printf("error opening , %s \n", strerror(errno));
		return -1;
	}
	//set_interface_attribs(fd, B115200, 0);
	while(1){
		char buf[50];
		int n = read(fd,buf,sizeof buf);
		for (int i=0; i<50; ++i){
			printf("%c",buf[i]);
		}
		printf("\n");
		for(int k=0; k<100000;++k);
	}

	return 0;
}

*/


