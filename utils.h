#include <stdio.h>
#include "defines.h"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

	//	STYPE mask = (STYPE)(-(v<0));
	//	int limit = ((v^mask)-mask)/128 + 1;
#define myabs(x) ((x^(-(x<0)))-(x<0))


STYPE max(STYPE a, STYPE b);

STYPE min(STYPE a, STYPE b);

STYPE sigadd(STYPE a, STYPE b);
//__attribute__((always_inline))
STYPE mult_sat(STYPE val, double mult);

void buff_volume_adjust(char * buff, int ini, int fin,double volume);

void printbuff(char* buff, int buff_size);

int vecToInt(char upper,char lower);

void quadunion(STYPE * buff, int buff_size, int A, int B, int Y, int power);

void bufftozero(char* buff, int ini, int buff_size);

void savebuff(char * buff_f, int buff_size_f, char * buff_d, int buff_size_d, int* pos);
