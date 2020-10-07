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

//-I /home/gabriell.araujo/LIBRARIES/upl/include/upl -L /home/gabriell.araujo/LIBRARIES/upl/lib -lupl -lm

/**
 * Comando de compilação:
 * g++ -Wall -O3 -I /home/gabriell.araujo/LIBRARIES/upl/include/upl -L /home/gabriell.araujo/LIBRARIES/upl/lib hist-omp.cpp -o main2 -lupl -lm -fopenmp
 *
 * Comando de execução
 * sudo ./main2 <data_set_number> <threads_number>
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
#include <sys/time.h>
#include <omp.h>
#include <upl.h>
#include <iostream>

#include "stddefines.h"

#define IMG_DATA_OFFSET_POS 10
#define BITS_PER_PIXEL_POS 28

typedef struct
{
        int red_thread[256];
		int green_thread[256];
		int blue_thread[256];
} DATA_THREAD;

DATA_THREAD* dados_threads;
int swap;      // to indicate if we need to swap byte order of header information
int red[256];
int green[256];
int blue[256];
int fd;
char* fdata;
struct stat finfo;
char* nome_data_set;
char* nome_interface;
double tempo_aux1, tempo_aux2 = 0.0;
double tempo_processa_dados = 0.0;
long long int corretude_red, corretude_green, corretude_blue, corretude_red_2, corretude_green_2, corretude_blue_2 = 0;
int NUMERO_DE_THREADS;
int n;

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
		printf("Swapping %d and %d\n", bytes[i], bytes[num_bytes - i - 1]);
		tmp = bytes[i];
		bytes[i] = bytes[num_bytes - i - 1];
		bytes[num_bytes - i - 1] = tmp;   
	}
}

int main(int argc, char *argv[]) 
{
	nome_interface = (char*) "openmp";
	n = atoll(argv[1]);
	NUMERO_DE_THREADS = atoll(argv[2]);
	define_data_set();

	// Make sure a filename is specified
	if (argv[1] == NULL) {
		printf("USAGE: %s <bitmap filename>\n", argv[0]);
		exit(1);
	}

	//fname = argv[1];

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
	dprintf("This file has %d bytes of image data, %d pixels\n", imgdata_bytes,
			imgdata_bytes / 3);

	dprintf("Starting sequential histogram\n");
	//printf("This file has %d bytes of image data, %d pixels\n", imgdata_bytes,num_pixels);	

	//set the number of threads that will be used
    omp_set_num_threads(NUMERO_DE_THREADS);
    //aloca vetor de dados para as threads
    dados_threads = (DATA_THREAD *) malloc(NUMERO_DE_THREADS * sizeof(DATA_THREAD));

	for (int i = 0; i < NUMERO_DE_THREADS; i++) 
	{
		for (int j = 0; j < 256; j++) 
		{
			dados_threads[i].red_thread[j] = 0;
			dados_threads[i].green_thread[j] = 0;
			dados_threads[i].blue_thread[j] = 0;
		}
	}

	memset(&(red[0]), 0, sizeof(int) * 256);
	memset(&(green[0]), 0, sizeof(int) * 256);
	memset(&(blue[0]), 0, sizeof(int) * 256);

	int size_aux = finfo.st_size;
	int init_aux = *data_pos; 

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

	#pragma omp parallel for schedule(static)
	for (int i= *data_pos; i < finfo.st_size; i+=3) 
	{
		int thread_id = omp_get_thread_num(); 

		//printf("[thread_%d==iteration_%d----calculate_%d]\n",thread_id,i,i);
		//printf("[thread_%d==iteration_%d----calculate_%d]\n",thread_id,i,i+1);
		//printf("[thread_%d==iteration_%d----calculate_%d]\n",thread_id,i,i+2);
      
		unsigned char *val = (unsigned char *)&(fdata[i]);
		dados_threads[thread_id].blue_thread[*val]++;

		val = (unsigned char *)&(fdata[i+1]);
		dados_threads[thread_id].green_thread[*val]++;

		val = (unsigned char *)&(fdata[i+2]);
		dados_threads[thread_id].red_thread[*val]++;
	}

	//coletando dados computados pelas threads
	for (int i = 0; i < NUMERO_DE_THREADS; i++) 
	{
		for (int j = 0; j < 256; j++) 
		{
			red[j] += dados_threads[i].red_thread[j];
			green[j] += dados_threads[i].green_thread[j];
			blue[j] += dados_threads[i].blue_thread[j];
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

	return 0;
}
