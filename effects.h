#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include "notes.h"
#include "defines.h"
#include "fft.h"
#include "lookuptable.h"

//new

int howmany;
char * echobuff;
int echobuff_size;
int echobuff_index;
//void echo(int period, char* buff, int buff_size,int ret, int layers);

void echo(char* buff, int buff_size, char* savedbuff, int savedbuff_size,
	  int savedbuff_pos, int bpm, double when, int howlong);

void buff_volume_adjust(char * buff, int ini, int fin,double volume);
int detectNote(char*buff, int buff_size,int rate); //TODO TE UN GRAN PROBLEMA QUAN FD NO DIVIDEIX A BUFF_SIZE
void distorsion(char* buff, int buff_size, double dis, int tipo);
void printbuff(char* buff, int buff_size);

void synth(int f,int instr, char* buff, int buff_size,int rate);

void chorus(int f, char * buff, int buff_size, int rate);

