#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include "notes.h"
#include "defines.h"
#include "lookuptable.h"

//new


void passeffect(char * buff, int buff_size, double sum);
void voleffect(char * buff, int buff_size, double sum, double intensity);
void lowhighpass(char * buff, int buff_size, double alpha);
void lowpass(char * buff, int buff_size, double alpha);
void highpass(char * buff, int buff_size, double alpha);

int howmany;
char * echobuff;
int echobuff_size;
int echobuff_index;
//void echo(int period, char* buff, int buff_size,int ret, int layers);

void echo(char* buff, int buff_size, char *savedbuff, int savedbuff_size, 
	  int savedbuff_pos, double interval, double volume);

//void echo(char* buff, int buff_size, char* savedbuff, int savedbuff_size,
//	  int savedbuff_pos, int bpm, double when, int howlong);

void reverb(char* buff, int buff_size, char* savedbuff, int savedbuff_size,
	  int savedbuff_pos);

int detectNote(/*char*buff, int buff_size,*/char* savedbuff, int savedbuff_size,
	int savedbuff_pos,int rate); 
void distorsion(char* buff, int buff_size, double dis, int tipo);

void newdist(char * buff, int buff_size, int th, double gain, int saturation, double reductor, double duracion, double ammount);

void lala(char* buff, int buff_size, double dis, int tipo);

void synth(int f,int instr, char* buff, int buff_size,int rate);

void chorus(int f, char * buff, int buff_size, int rate);
void flanger(char * buff, int buff_size, char * savedbuff, int savedbuff_size, 
	     int pos_in_savedbuff, int bpm);
//void lowpass(char* buff, int buff_size, int ammount);
