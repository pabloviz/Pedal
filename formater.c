#include <stdio.h>
#include <unistd.h>


int main(){

	char buff[4];
	short * sbuff = (short*)(&buff[0]);
	while(read(0,buff,4)){
		printf("%d\n",sbuff[0]);
	}
}
