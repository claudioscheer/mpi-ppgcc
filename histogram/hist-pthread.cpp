/* Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Stanford University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

/**
 * Comando de compilação:
 * g++ -Wall -O3 -I /home/gabriell.araujo/LIBRARIES/upl/include/upl -L /home/gabriell.araujo/LIBRARIES/upl/lib hist-pthread.cpp -o main4 -lupl -lm -lpthread
 *
 * Comando de execução
 * ./main5 <data_set_number> <number_of_threads>
 *
 * Imagens:
 * small, medium e large
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include <upl.h>
#include <iostream>

#include "stddefines.h"

#define IMG_DATA_OFFSET_POS 10
#define BITS_PER_PIXEL_POS 28

int swap;      // to indicate if we need to swap byte order of header information
int red[256];
int green[256];
int blue[256];
int fd;
char *fdata;
struct stat finfo;
char* nome_data_set;
char* nome_interface;
double tempo_aux1, tempo_aux2 = 0.0;
double tempo_processa_dados = 0.0;
long long int corretude_red, corretude_green, corretude_blue, corretude_red_2, corretude_green_2, corretude_blue_2 = 0;
int NUMERO_DE_THREADS;
int n;

typedef struct {
	unsigned char *data;
	long data_pos;
	long data_len;
	int red[256];
	int green[256];
	int blue[256];
} thread_arg_t;

void define_data_set()
{
	if(n == 1)
	{
		nome_data_set = (char*) "small.bmp";
	}
	if(n == 2)
	{
		nome_data_set = (char*) "med.bmp";
	}
	if(n == 3)
	{
		nome_data_set = (char*) "large.bmp";
	}
}

void gerar_log()
{
    printf("PERFORMANCE:\n");
    printf("dataset: %s\n", nome_data_set);
	printf("interface: %s\n", nome_interface);
	printf("numero_de_threads: %d\n", NUMERO_DE_THREADS);
	printf("tempo_de_execucao_em_segundos: %lf\n", tempo_processa_dados);
	printf("corretude_do_algoritmo: ");
	printf("red=%lld", corretude_red);
	printf("_red_2=%lld", corretude_red_2);
	printf("_green=%lld", corretude_green);
	printf("_green_2=%lld", corretude_green_2);
	printf("_blue=%lld", corretude_blue);
    printf("_blue_2=%lld\n", corretude_blue_2);
}

double tempo_atual()
{
	struct timeval tempoValor;
	gettimeofday(&tempoValor, 0);
	return tempoValor.tv_sec + (tempoValor.tv_usec / 1e6);
}

void calcula_corretude()
{
	for(int i = 0; i < 128; i++)
	{
		corretude_red += red[i];
		corretude_green += green[i];
		corretude_blue += blue[i];
	}
	for(int i = 128; i < 256; i++)
	{
		corretude_red_2 += red[i];
		corretude_green_2 += green[i];
		corretude_blue_2 += blue[i];
	}
}

/* test_endianess
 *
 */
void test_endianess() {
	unsigned int num = 0x12345678;
	char *low = (char *)(&(num));
	if (*low ==  0x78) {
		dprintf("No need to swap\n");
		swap = 0;
	}
	else if (*low == 0x12) {
		dprintf("Need to swap\n");
		swap = 1;
	}
	else {
		printf("Error: Invalid value found in memory\n");
		exit(1);
	} 
}

/* swap_bytes
 *
 */
void swap_bytes(char *bytes, int num_bytes) {
	int i;
	char tmp;

	for (i = 0; i < num_bytes/2; i++) {
		dprintf("Swapping %d and %d\n", bytes[i], bytes[num_bytes - i - 1]);
		tmp = bytes[i];
		bytes[i] = bytes[num_bytes - i - 1];
		bytes[num_bytes - i - 1] = tmp;   
	}
}

/* calc_hist
 * Function that computes the histogram for the region
 * assigned to each thread
 */
void *calc_hist(void *arg) 
{   
	int *red;
   	int *green;
   	int *blue;
	int i;
	thread_arg_t *thread_arg = (thread_arg_t *)arg;
	unsigned char *val;
	
	//int* red = (int *)calloc(256, sizeof(int));
	//int* green = (int *)calloc(256, sizeof(int));
	//int* blue = (int *)calloc(256, sizeof(int));

	red = thread_arg->red;
	green = thread_arg->green;
	blue = thread_arg->blue;


	//printf("Starting at %ld, doing %ld bytes\n", thread_arg->data_pos, thread_arg->data_len);
	for (i= thread_arg->data_pos; i < thread_arg->data_pos + thread_arg->data_len; i+=3) {

		val = &(thread_arg->data[i]);
		blue[*val]++;

		val = &(thread_arg->data[i+1]);
		green[*val]++;

		val = &(thread_arg->data[i+2]);
		red[*val]++;   
	}
	/*
	   thread_arg->red = red;
	   thread_arg->green = green;
	   thread_arg->blue = blue;
	 */
	return (void *)0;
}


int main(int argc, char *argv[]) 
{
	nome_interface = (char*) "pthread";
	n = atoll(argv[1]);
	NUMERO_DE_THREADS = atoll(argv[2]);
	define_data_set();

	pthread_t *pid;
	pthread_attr_t attr;
	thread_arg_t *arg;
	int num_per_thread;
	int excess;

	// Make sure a filename is specified
	if (argv[1] == NULL) {
		printf("USAGE: %s <bitmap filename>\n", argv[0]);
		exit(1);
	}

	// Read in the file
	CHECK_ERROR((fd = open(nome_data_set, O_RDONLY)) < 0);
	// Get the file info (for file length)
	CHECK_ERROR(fstat(fd, &finfo) < 0);
	// Memory map the file
	CHECK_ERROR((fdata = (char*) mmap(0, finfo.st_size + 1, 
					PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == NULL);

	if ((fdata[0] != 'B') || (fdata[1] != 'M')) {
		printf("File is not a valid bitmap file. Exiting\n");
		exit(1);
	}

	test_endianess();    // will set the variable "swap"

	unsigned short *bitsperpixel = (unsigned short *)(&(fdata[BITS_PER_PIXEL_POS]));
	if (swap) {
		swap_bytes((char *)(bitsperpixel), sizeof(*bitsperpixel));
	}
	if (*bitsperpixel != 24) {    // ensure its 3 bytes per pixel
		printf("Error: Invalid bitmap format - ");
		printf("This application only accepts 24-bit pictures. Exiting\n");
		exit(1);
	}

	unsigned short *data_pos = (unsigned short *)(&(fdata[IMG_DATA_OFFSET_POS]));
	if (swap) {
		swap_bytes((char *)(data_pos), sizeof(*data_pos));
	}

	int imgdata_bytes = (int)finfo.st_size - (int)(*(data_pos));
	int num_pixels = ((int)finfo.st_size - (int)(*(data_pos))) / 3;
	dprintf("This file has %d bytes of image data, %d pixels\n", imgdata_bytes,num_pixels);

	dprintf("Starting pthreads histogram\n");

	memset(&(red[0]), 0, sizeof(int) * 256);
	memset(&(green[0]), 0, sizeof(int) * 256);
	memset(&(blue[0]), 0, sizeof(int) * 256);

	/* Set a global scope */
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	//CHECK_ERROR((NUMERO_DE_THREADS = sysconf(_SC_NPROCESSORS_ONLN)) <= 0);
	num_per_thread = num_pixels / NUMERO_DE_THREADS;
	excess = num_pixels % NUMERO_DE_THREADS;

	CHECK_ERROR( (pid = (pthread_t *)malloc(sizeof(pthread_t) * NUMERO_DE_THREADS)) == NULL);
	CHECK_ERROR( (arg = (thread_arg_t *)calloc(sizeof(thread_arg_t), NUMERO_DE_THREADS)) == NULL);

    long curr_pos = (long)(*data_pos);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //INICIO-PROCESSAMENTO///////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    //###############################################################
    //cache miss
    int fd_cache;
	if(UPL_init_cache_miss_monitoring(&fd_cache) == 0)
    {
        std::cout << "************* Error when UPL_init_cache_miss_monitoring(...)" << std::endl;
	}
    //energia
    int *rapl_fd = new int[4];
    if(UPL_init_count_rapl(rapl_fd) == 0)
    {
        std::cout << "************* Error when UPL_init_count_rapl(...)" << std::endl;
    }
    //###############################################################

    tempo_aux1 = tempo_atual();

	/* Assign portions of the image to each thread */	
	for (int i = 0; i < NUMERO_DE_THREADS; i++) 
    {
		arg[i].data = (unsigned char *)fdata;
		arg[i].data_pos = curr_pos;
		arg[i].data_len = num_per_thread;
		if (excess > 0) 
        {
			arg[i].data_len++;
			excess--;
		}

		arg[i].data_len *= 3;   // 3 bytes per pixel
		curr_pos += arg[i].data_len;

		pthread_create(&(pid[i]), &attr, calc_hist, (void *)(&(arg[i])));   
	}

	for (int i = 0; i < NUMERO_DE_THREADS; i++) 
    {
		pthread_join(pid[i] , NULL);   
	}

	for (int i = 0; i < NUMERO_DE_THREADS; i++) 
    {
		for (int j = 0; j < 256; j++) 
        {
			red[j] += arg[i].red[j];
			green[j] += arg[i].green[j];
			blue[j] += arg[i].blue[j];
		}
	}

	tempo_aux2 = tempo_atual();
	tempo_processa_dados = tempo_aux2 - tempo_aux1;

    //###############################################################
    //cache miss
    printf("CACHE MISS:\n");
    long long r_cache = UPL_get_cache_miss(fd_cache);
	if(r_cache < 0)
    {
        std::cout << "************* Error when UPL_get_cache_miss(...)" << std::endl;
	}
	std::cout << "UPLib -> Total cache-miss(KB): " << r_cache << std::endl;
    //energia
    printf("ENERGY:\n");
    if(UPL_finalize_count_rapl(rapl_fd) == 0){
        std::cout << "************* Error when UPL_finalize_count_rapl(...)" << std::endl;
    }
    delete rapl_fd;
    //informacoes finais
    printf("FINAL DATA:\n");
    printf("UPLib -> Memory usage (KB): %ld \n", UPL_getProcMemUsage());
    printf("UPLib -> Threads used: %ld \n", UPL_getProcTotThreads());
    printf("UPLib -> Voluntary_ctx_switches: %ld \n", UPL_getProcVoluntary_ctx_switches());
    printf("UPLib -> NonVoluntary_ctx_switches: %ld \n", UPL_getProcNonVoluntary_ctx_switches());
    //###############################################################

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //FIM-PROCESSAMENTO//////////////////////////////////////////////////////////////////////////////////////	
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

	calcula_corretude();
	gerar_log();

	dprintf("\n\nBlue\n");
	dprintf("----------\n\n");
	for (int i = 0; i < 256; i++) {
		dprintf("%d - %d\n", i, blue[i]);        
	}

	dprintf("\n\nGreen\n");
	dprintf("----------\n\n");
	for (int i = 0; i < 256; i++) {
		dprintf("%d - %d\n", i, green[i]);        
	}

	dprintf("\n\nRed\n");
	dprintf("----------\n\n");
	for (int i = 0; i < 256; i++) {
		dprintf("%d - %d\n", i, red[i]);        
	}

	CHECK_ERROR(munmap(fdata, finfo.st_size + 1) < 0);
	CHECK_ERROR(close(fd) < 0);

	free(pid);
	//for(int i = 0; i < NUMERO_DE_THREADS; i++) {
	//	free(arg[i].red);
	//	free(arg[i].green);
	//	free(arg[i].blue);
	//}
	free(arg);
	pthread_attr_destroy(&attr);

	return 0;
}
