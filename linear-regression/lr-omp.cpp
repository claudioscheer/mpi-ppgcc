/** Copyright (c) 2007, Stanford University
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
 * DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY  BE LIABLE FOR ANY
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
 * Este programa é uma implementação do algoritmo de regressão linear.
 *
 * Comando de compilação:
 * g++ -O3 lr-omp.cpp -fopenmp -o main2
 *
 * Comando de execução:
 * ./main2 <numero_do_data_set> <numero_de_threads>
 *
 * Sugestão de número máximo alocável:
 * 1000000 (um milhão, que ocupa ~= 15 megabytes da memória) 
 *
 * Sugestão de execução:
 * ./main2 1 1000000 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <upl.h>
#include <iostream>

using namespace std;

typedef struct
{
	long long x;
	long long y;
} POINT_T;

typedef struct
{
        long long sx;
        long long sy;
        long long sxx;
        long long syy;
        long long sxy;
} DATA_THREAD;

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
DATA_THREAD* dados_threads;
int NUMERO_DE_THREADS;

//assinatura de funções
double tempoAtual();

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

	#pragma omp parallel for schedule(static)
	for(long long i = 0; i < maximo_alocavel; i++)
    	{
        	int thread_id = omp_get_thread_num(); 
        				
        	int x_aux = points[i].x;
			int y_aux = points[i].y;       	

        	//zona crítica
        	dados_threads[thread_id].sx += x_aux;
        	dados_threads[thread_id].sxx += x_aux * x_aux;
        	dados_threads[thread_id].sy += y_aux;
        	dados_threads[thread_id].syy += y_aux * y_aux;
        	dados_threads[thread_id].sxy += x_aux * y_aux;
        }
	
	//coletando dados computados pelas threads
	for(int k = 0; k < NUMERO_DE_THREADS; k++)
	{
		SX_ll += dados_threads[k].sx;
		SY_ll += dados_threads[k].sy;
		SXX_ll += dados_threads[k].sxx;
		SYY_ll += dados_threads[k].syy;
		SXY_ll += dados_threads[k].sxy;
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

	SX = (double)SX_ll;
	SY = (double)SY_ll;
	SXX = (double)SXX_ll;
	SYY = (double)SYY_ll;
	SXY = (double)SXY_ll;

	b = (double)(n*SXY - SX*SY) / (n*SXX - SX*SX);
	a = (SY_ll - b*SX_ll) / n;
	xbar = (double)SX_ll / n;
	ybar = (double)SY_ll / n;
	r2 = (double)(n*SXY - SX*SY) * (n*SXY - SX*SY) / ((n*SXX - SX*SX)*(n*SYY - SY*SY));	

	free(points);
}

int main(int argc, char *argv[])
{ 
	nome_interface = (char*) "openmp";
	n = atoll(argv[1]);
	NUMERO_DE_THREADS = atoll(argv[2]);
	define_data_set();

	//set the number of threads that will be used
    	omp_set_num_threads(NUMERO_DE_THREADS);
    	//aloca vetor de dados para as threads
    	dados_threads = (DATA_THREAD *) malloc(NUMERO_DE_THREADS * sizeof(DATA_THREAD));

	//inicializa vetor de dados para as threads
	for(long long i = 0; i < NUMERO_DE_THREADS; i++)
    	{
        	dados_threads[i].sx = 0;
        	dados_threads[i].sxx = 0;
        	dados_threads[i].sy = 0;
        	dados_threads[i].syy = 0;
        	dados_threads[i].sxy = 0;
    	}

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
