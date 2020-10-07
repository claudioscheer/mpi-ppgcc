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
 * PUCRS - FACIN - GMAP
 *
 * Modificações no código: (Gabriell Araujo)
 * Versão: (22/04/2016)
 *
 * Comandos de compilação:
 * g++ -O3 lr-pthread.cpp -lpthread -o main4
 *
 * Comando de execução:
 * ./main4 <numero_do_data_set> <numero_de_threads>
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/time.h>
#include <upl.h>
#include <iostream>

typedef struct {
	char x;
	char y;
} POINT_T;

typedef struct
{
	pthread_t tid;
	POINT_T *points;
	int num_elems;
	long long SX;
	long long SY; 
	long long SXX;
	long long SYY; 
	long long SXY;
} lreg_args;

//variáveis globais
long long SX_ll = 0, SY_ll = 0, SXX_ll = 0, SYY_ll = 0, SXY_ll = 0;
double xbar, ybar, r2, SX, SY, SXX, SYY, SXY;
POINT_T* points;
long long n;
double a, b;
char* nome_data_set;
char* nome_interface;
long long maximo_alocavel;
double tempo_aux1 = 0.0;
double tempo_aux2 = 0.0;
double tempo_carrega_dados = 0.0;
double tempo_processa_dados = 0.0;
int NUMERO_DE_THREADS;

void *linear_regression_pthread(void *args_in) 
{
	lreg_args* args =(lreg_args*)args_in;
	int i;

	args->SX = 0;
	args->SXX = 0;
	args->SY  = 0;
	args->SYY = 0;
	args->SXY = 0;

	// ADD UP RESULTS
	for (i = 0; i < args->num_elems; i++)
	{
		//Compute SX, SY, SYY, SXX, SXY
		args->SX  += args->points[i].x;
		args->SXX += args->points[i].x*args->points[i].x;
		args->SY  += args->points[i].y;
		args->SYY += args->points[i].y*args->points[i].y;
		args->SXY += args->points[i].x*args->points[i].y;
	}

	return (void *)0;
}

/**
 * A partir do parâmetro n que foi passado pelo main, o algoritmo determina
 * qual é o nome do data_set que vai ser usado.
 */
void define_data_set()
{
	if(n == 1)
	{
		nome_data_set = (char*) "lr_data_set_1.txt";
	}
	if(n == 2)
	{
		nome_data_set = (char*) "lr_data_set_2.txt";
	}
	if(n == 3)
	{
		nome_data_set = (char*) "lr_data_set_3.txt";
	}
	if(n == 4)
	{
		nome_data_set = (char*) "lr_data_set_4.txt";
	}
}

long long calculaY(long long x)
{
	return (x * a) + b;
}

void gerar_log()
{
    printf("PERFORMANCE:\n");
    printf("dataset: %s\n", nome_data_set);
	printf("interface: %s\n", nome_interface);
	printf("numero_de_threads: %d\n", NUMERO_DE_THREADS);
    printf("tempo_de_execucao_em_segundos: %lf\n", tempo_processa_dados);
	printf("corretude_do_algoritmo: ");
	printf("_sx:%lld", SX_ll);
	printf("_sy:%lld", SY_ll);
	printf("_sxx:%lld", SXX_ll);
	printf("_syy:%lld", SYY_ll);
	printf("_sxy:%lld\n", SXY_ll);
}

/**
 * Coleta o tempo atual do programa em segundos.
 */
double tempoAtual()
{
	struct timeval tempoValor;
	gettimeofday(&tempoValor, 0);
	return tempoValor.tv_sec + (tempoValor.tv_usec / 1e6);
}

void carregarDadosDoArquivo()
{
	FILE* arquivo;

	arquivo = fopen(nome_data_set, "rt");

	if(arquivo == NULL)
	{
		printf("Erro ao tentar abrir o arquivo!\n");
	}
	else
	{
		//faz leitura da quantidade de pontos existentes no arquivo
		bool b = fscanf(arquivo, "%lld", &n);

		points = (POINT_T *) malloc(n * sizeof(POINT_T));

		long long x = 0;
		long long y = 0;

		//carrega dados na memóra
		for(long long i = 0; i < n; i++)
		{
			bool b2 = fscanf(arquivo, "%lld %lld", &x, &y);
			points[i].x = x;
			points[i].y = y;
		}

		fclose(arquivo);
	}	
}

void executar_algoritmo()
{
	int req_units, i;
	pthread_attr_t attr;
	lreg_args* tid_args;

	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	req_units = maximo_alocavel / NUMERO_DE_THREADS;
	tid_args = (lreg_args *)malloc(NUMERO_DE_THREADS * sizeof(lreg_args)); 

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //INÍCIO DA EXECUÇÃO DO ALGORITMO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    tempo_aux1 = tempoAtual();

	//aloca e executa threads
	for(i = 0; i < NUMERO_DE_THREADS; i++)
	{
		tid_args[i].points = &points[i*req_units];
		tid_args[i].num_elems = req_units;

		if(i == (NUMERO_DE_THREADS - 1))
		{
			tid_args[i].num_elems = n - i*req_units;
		}

		pthread_create(&tid_args[i].tid, &attr, linear_regression_pthread, (void*)&tid_args[i]);
	}	

	//sincroniza threads
	for (i = 0; i < NUMERO_DE_THREADS; i++)
	{
		int ret_val;
		pthread_join(tid_args[i].tid, (void **)(void*)&ret_val);
		ret_val != 0;

		SX_ll += tid_args[i].SX;
		SY_ll += tid_args[i].SY; 
		SXX_ll += tid_args[i].SXX; 
		SYY_ll += tid_args[i].SYY; 
		SXY_ll += tid_args[i].SXY;
	}	

	tempo_aux2 = tempoAtual();
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////
    //TÉRMINO DA EXECUÇÃO DO ALGORITMO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //////////////////////////////////////////////////////////////////////////////////////////////////////// 

    free(tid_args);

	a, b, xbar, ybar, r2;
	SX = (double)SX_ll;
	SY = (double)SY_ll;
	SXX= (double)SXX_ll;
	SYY= (double)SYY_ll;
	SXY= (double)SXY_ll;

	b = (double)(n*SXY - SX*SY) / (n*SXX - SX*SX);
	a = (SY_ll - b*SX_ll) / n;
	xbar = (double)SX_ll / n;
	ybar = (double)SY_ll / n;
	r2 = (double)(n*SXY - SX*SY) * (n*SXY - SX*SY) / ((n*SXX - SX*SX)*(n*SYY - SY*SY));	

	free(points);
}

int main(int argc, char *argv[])
{ 
	nome_interface = (char*) "pthread";
	n = atoll(argv[1]);
	NUMERO_DE_THREADS = atoll(argv[2]);
	define_data_set();

	//TEMPO CARREGA ARQUIVO
	tempo_aux1 = tempoAtual();

	carregarDadosDoArquivo();
	maximo_alocavel = n;

	tempo_aux2 = tempoAtual();
	tempo_carrega_dados = tempo_aux2 - tempo_aux1;
	//TEMPO CARREGA ARQUIVO

	executar_algoritmo();	

	gerar_log();

	return 0;
}
