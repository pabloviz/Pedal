#include <stdio.h>
#include "defines.h"




STYPE max(STYPE a, STYPE b);

STYPE min(STYPE a, STYPE b);

STYPE sigadd(STYPE a, STYPE b);

int vecToInt(char upper,char lower);

void quadunion(STYPE * buff, int buff_size, int A, int B, int Y, int power);

void bufftozero(char* buff, int ini, int buff_size);

void savebuff(char * buff_f, int buff_size_f, char * buff_d, int buff_size_d, int* pos);
