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
 * Versão: (19/01/2015)
 *
 * Esta é uma implementação do algoritmo K-MEANS.
 *
 * Código-fonte base: https://github.com/kozyraki/phoenix
 *
 * Comandos de compilação:
 * g++ -O3 kmeans-seq.cpp -o main1 -I /home/gabriell.araujo/LIBRARIES/upl/include/upl -L /home/gabriell.araujo/LIBRARIES/upl/lib -lupl -lm
 *
 * Comandos de execução:
 * .main1 <numero_data_set> 
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <upl.h>
#include <iostream>

using namespace std;

//variáveis globais
char* nome_data_set; //armazena o nome do data_set usado
int numero_data_set; //número do data_set
int numero_threads; //número de threads a serem usadas pela execução do programa
int num_points; //número de pontos
int num_means; //número de centróides
int dim; //dimensão dos pontos
int** points; //vetor de pontos
int** means; //vetor de centróides
int* clusters; //vetor vetor com índice dos centróides de cada ponto
int modified; //controla a existência de modificações
int grid_size; //os números variam de zero até grid_size
long long int controle_algoritmo; //controla quantas vezes o laço do algoritmo é executado
double tempo_total_de_execucao; //armazena o tempo total de execução do algoritmo
char* nome_interface; //nome da interface usada

/**
 * A partir do parâmetro numero_data_set que foi passado pelo main, o algoritmo determina
 * qual é o nome do data_set que vai ser usado.
 */
void define_data_set()
{
	if(numero_data_set == 1)
	{
		nome_data_set = (char*) "data_set_1.txt";
	}
	if(numero_data_set == 2)
	{
		nome_data_set = (char*) "data_set_2.txt";
	}
	if(numero_data_set == 3)
	{
		nome_data_set = (char*) "data_set_3.txt";
	}
}

/**
 * Faz leitura dos dados do data_set.
 */
void ler_arquivo()
{
	FILE* arquivo;  

	arquivo = fopen(nome_data_set, "rt");

	if(arquivo == NULL)
	{
		printf("Erro ao tentar abrir o arquivo!\n");
	}
	else
	{
		bool saida_leitura;
		int valor_lido;
		char palavra[20];		

		//lendo o número de pontos
		saida_leitura = fscanf(arquivo, "%s %d", palavra, &num_points);

		//lendo o número de centróides
		saida_leitura = fscanf(arquivo, "%s %d", palavra, &num_means);

		//lendo a dimensão dos pontos 
		saida_leitura = fscanf(arquivo, "%s %d", palavra, &dim);

		//lendo o título do bloco de pontos
		saida_leitura = fscanf(arquivo, "%s", palavra);

		//alocando vetor de pontos
		points = (int**) malloc(sizeof(int*) * num_points);
		for (int i = 0; i<num_points; i++)
		{
			points[i] = (int*) malloc(sizeof(int) * dim);
		}

		//preenchendo o vetor de pontos
		for(int i = 0; i < num_points; i++)
		{
			for(int j = 0; j < dim; j++)
			{
				saida_leitura = fscanf(arquivo, "%d", &valor_lido);
				points[i][j] = valor_lido;
			}
		}

		//lendo o título do bloco de centróides
		saida_leitura = fscanf(arquivo, "%s", palavra);

		//alocando vetor de centróides
		means = (int**) malloc(sizeof(int*) * num_means);
		for (int i = 0; i<num_means; i++)
		{
			means[i] = (int*) malloc(sizeof(int) * dim);
		}

		//preenchendo o vetor de centróides
		for(int i = 0; i < num_means; i++)
		{
			for(int j = 0; j < dim; j++)
			{
				saida_leitura = fscanf(arquivo, "%d", &valor_lido);
				means[i][j] = valor_lido;
				//printf("%d ", valor_lido);	
			}
			//printf("\n");	
		}

		//alocando vetor que guarda o índice do centróide de cada ponto
		clusters = (int*) malloc(sizeof(int) * num_points);
		memset(clusters, -1, sizeof(int) * num_points);

		//fecha o arquivo
		fclose(arquivo);
	}	
}

/**
 * Escreve arquivo de log.
 */
void gerar_log()
{
    printf("PERFORMANCE:\n");
    printf("dataset: %s\n", nome_data_set);
	printf("interface: %s\n", nome_interface);
	printf("numero_de_threads: %d\n", numero_threads);
    printf("tempo_de_execucao_em_segundos: %lf\n", tempo_total_de_execucao);
	printf("corretude_do_algoritmo: ");
	printf("iteracoes+:%lld\n", controle_algoritmo);
}

/**
 *  Calcula a distância entre dois pontos.
 */
inline unsigned int get_sq_dist(int *v1, int *v2)
{
	unsigned int sum = 0;

	for(int i = 0; i < dim; i++)
	{
		sum += ((v1[i] - v2[i]) * (v1[i] - v2[i]));
	}
	return sum;
}

/**
 * Helper function to update the total distance sum.
 */
void add_to_sum(int *sum, int *point)
{
	for (int i = 0; i < dim; i++)
	{
		sum[i] += point[i];
	}
}

/**
 * Imprime o resultado da execução do algoritmo.
 */
void imprimir_pontos(int **points, int **means, int *clusters)
{
	unsigned int min_dist, cur_dist;
	int min_idx;

	for (int i = 0; i < num_points; i++)
	{
		min_dist = get_sq_dist(points[i], means[0]);
		min_idx = 0;

		for (int j = 1; j < num_means; j++)
		{
			cur_dist = get_sq_dist(points[i], means[j]);
			if (cur_dist < min_dist)
			{
				min_dist = cur_dist;
				min_idx = j;
			}
		}

		//imprime ponto
		printf("ponto: [");
		for(int k = 0; k < dim; k++)
		{
			printf(" %d ", points[i][k]);
		}
		printf("]");
		//imprime centróide mais próximo do ponto
		printf(" ------ centróide: [");
		for(int z = 0; z < dim; z++)
		{
			printf(" %d ", means[min_idx][z]);
		}
		printf("]\n");
	}
}

/**
 * Pega o tempo atual do programa em segundos.
 */
double tempo_atual()
{
	//estrutura para guardar informações do tempo de execução
	struct timeval tempoValor;

	//calcula o tempo atual e guarda em tempoValor
	gettimeofday(&tempoValor, 0);

	//*tv_sec são segundos
	//*tv_usec são microsegundos

	//retorna o total de segundos do tempo atual.
	return tempoValor.tv_sec + (tempoValor.tv_usec / 1e6);
}

/**
 * Calcula o tempo de execução a partir dos parâmetros.
 */
double tempo_total(double start, double end)
{
	return end - start;
}

int main(int argc, char **argv)
{
	numero_data_set = atoi(argv[1]);
	numero_threads = 1;
	nome_interface = (char*) "sequencial";
	define_data_set();
	ler_arquivo();
	controle_algoritmo = 0;
	modified = true;		
	
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

    double inicio = tempo_atual();

	while (modified)
	{
		modified = false;

		//////////****definindo o centróide de cada ponto
		for (int i = 0; i < num_points; i++)
		{
			unsigned int min_dist = get_sq_dist(points[i], means[0]);
        		int min_idx = 0;

			for (int j = 1; j < num_means; j++)
			{
				unsigned int cur_dist = get_sq_dist(points[i], means[j]);
				if (cur_dist < min_dist)
				{
					min_dist = cur_dist;
					min_idx = j;
				}
			}

			if (clusters[i] != min_idx)
			{
				clusters[i] = min_idx;
				modified = true;
			}
		}

		//////////****atualizando centróides
		for (int i = 0; i < num_means; i++)
		{
			int* sum = (int *)malloc(dim * sizeof(int));
        		memset(sum, 0, dim * sizeof(int));
        		int grp_size = 0;

			for (int j = 0; j < num_points; j++)
			{
				if (clusters[j] == i)
				{
					add_to_sum(sum, points[j]);
					grp_size++;
				}
			}

			for (int j = 0; j < dim; j++)
			{
				if (grp_size != 0)
				{
					means[i][j] = sum[j] / grp_size;
				}
			}
		
			free(sum);
		}

		controle_algoritmo++;
	}	

    double fim = tempo_atual();	
	tempo_total_de_execucao = tempo_total(inicio, fim);		

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

	//imprimindo variável de controle do algoritmo e o tempo de execução do mesmo
	//printf("Quantidade de vezes em que o laço while foi executado: %lld\n",controle_algoritmo);
	//printf("Tempo total de execução do algoritmo: %lf segundos\n\n", tempo_total_de_execucao);

	//imprime o resultado final do algoritmo, associando cada ponto ao seu centróide final
	//imprimir_pontos(points, means, clusters);

	//gerar log de execução
	gerar_log();

	//desalocando vetor de pontos
	for (int i = 0; i<num_points; i++)
	{
		free(points[i]);
	}
	free(points);

	//desalocando vetor de centróides
	for (int i = 0; i<num_means; i++)
	{
		free(means[i]);
	}
	free(means);

	return 0;
}
