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
 * DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY  BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

/**
 * comando para compilar:
 * g++ -O3 pca-seq.cpp -o main1
 *
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <upl.h>
#include <iostream>

int num_rows, num_cols;
int **matrix, **cov;
int *mean;
char* nome_data_set;
char* nome_interface;
double tempo_aux1, tempo_aux2 = 0.0;
double tempo_processa_dados = 0.0;
long long int corretude_mean, corretude_cov = 0;
int NUMERO_DE_THREADS;
int n;


void define_data_set()
{
	if(n == 1)
	{
		nome_data_set = (char*) "pca_data_1.txt";
	}
	if(n == 2)
	{
		nome_data_set = (char*) "pca_data_2.txt";
	}
	if(n == 3)
	{
		nome_data_set = (char*) "pca_data_3.txt";
	}
}

void gerar_log()
{
    printf("PERFORMANCE:\n");
    printf("dataset: %s\n", nome_data_set);
	printf("interface: %s\n", nome_interface);
	printf("numero_de_threads: %d\n", 1);
    printf("tempo_de_execucao_em_segundos: %lf\n", tempo_processa_dados);
	printf("corretude_do_algoritmo: ");
    printf("mean+:%lld", corretude_mean);
	printf("_cov+:%lld\n", corretude_cov);
}

double tempo_atual()
{
	struct timeval tempoValor;
	gettimeofday(&tempoValor, 0);
	return tempoValor.tv_sec + (tempoValor.tv_usec / 1e6);
}

void carregar_dados_do_arquivo()
{
	FILE* arquivo;

	arquivo = fopen(nome_data_set, "rt");

	if(arquivo == NULL)
	{
		printf("Erro ao tentar abrir o arquivo!\n");
	}
	else
	{
		//faz leitura da quantidade de dados existentes no arquivo
		fscanf(arquivo, "%d", &num_rows);
		fscanf(arquivo, "%d", &num_cols);		

		//aloca matriz
		matrix = (int **)malloc(sizeof(int *) * num_rows);
		for (int i=0; i<num_rows; i++) 
		{
			matrix[i] = (int *)malloc(sizeof(int) * num_cols);
		}

		//carrega dados do arquivo
		for (int i=0; i<num_rows; i++) 
		{
			for (int j=0; j<num_cols; j++) 
			{
				fscanf(arquivo, "%d", &matrix[i][j]);
			}
		}

		fclose(arquivo);
	}	
}

/** dump_points()
 *  Print the values in the matrix to the screen
 */
void dump_points()
{
	for (int i = 0; i < num_rows; i++) 
	{
		for (int j = 0; j < num_cols; j++)
		{
			printf("%5d ",matrix[i][j]);
		}
		printf("\n");
	}
}

/** calc_mean()
 *  Compute the mean for each row
 */
void calc_mean() 
{
	int sum = 0;

	for (int i = 0; i < num_rows; i++) 
	{
		sum = 0;
		for (int j = 0; j < num_cols; j++) 
		{
			sum += matrix[i][j];
		}
		mean[i] = sum / num_cols;   
	}
}

/** calc_cov()
 *  Calculate the covariance
 */
void calc_cov() 
{
	int sum;

	for (int i = 0; i < num_rows; i++) 
	{
		for (int j = i; j < num_rows; j++) 
		{
			sum = 0;
			for (int k = 0; k < num_cols; k++) 
			{
				sum = sum + ((matrix[i][k] - mean[i]) * (matrix[j][k] - mean[j]));
			}
			cov[i][j] = cov[j][i] = sum/(num_cols-1);
		}   
	}   
}

void calcula_corretude()
{
	for(int i = 0; i < num_rows; i++)
	{
		corretude_mean += (long long int) mean[i];

		for(int j = 0; j < num_cols; j++)
		{
			corretude_cov += (long long int) cov[i][j];
		}
	}
	
}

int main(int argc, char **argv) 
{
	nome_interface = (char*) "sequencial";
	n = atoll(argv[1]);
	//NUMERO_DE_THREADS = atoll(argv[2]);
	define_data_set();		

	carregar_dados_do_arquivo();

	// Allocate Memory to store the mean and the covariance matrix
	mean = (int *)malloc(sizeof(int) * num_rows);
	cov = (int **)malloc(sizeof(int *) * num_rows);
	for (int i=0; i<num_rows; i++) 
	{
		cov[i] = (int *)malloc(sizeof(int) * num_rows);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//inicio processamento////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	calc_mean();
	calc_cov();
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
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//fim processamento///////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

	calcula_corretude();

	gerar_log();

	return 0;
}
