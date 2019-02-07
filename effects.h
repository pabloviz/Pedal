#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


//new

int howmany;
char * echobuff;
int echobuff_size;
int echobuff_index;
void echo(int period, char* buff, int buff_size,int ret, int layers);


char volume_adjust(char c, double volume);
