#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "defines.h"

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)
#define BCM2708_PERI_BASE 0x3F000000
#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000)


int mem_fd;
void * gpio_map;
volatile unsigned *gpio;

int gpio_ini();

void gpio_setOutput(int pin_num);
void gpio_setInput(int pin_num);
void gpio_setValue(int pin_num, int value);

int gpio_readValue(int pin_num);
void gpio_clear();
