#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "notes.h"
#define PI 3.141596

//new

int howmany;
char * echobuff;
int echobuff_size;
int echobuff_index;
void echo(int period, char* buff, int buff_size,int ret, int layers);


char volume_adjust(char * buff, int ini, int fin,double volume);
int detectNote(char*buff, int buff_size,int rate); //TODO TE UN GRAN PROBLEMA QUAN FD NO DIVIDEIX A BUFF_SIZE
void printbuff(char* buff, int buff_size);

void synth(int f, int instr, char* buff, int buff_size,int rate);

