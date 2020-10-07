/**
 * PUCRS - FACIN - GMAP
 *
 * Modificações no código: (Gabriell Araujo)
 * Versão: (18/01/2016)
 *
 * Este programa gera os data_sets para execução do algoritmo kmeans.
 *
 * Comando de compilação:
 * g++ -O3 gerador_data_set.cpp -o gerador_data_set
 *
 * Comando de execução:
 * ./gerador_data_set <num_points_data_set_1> <num_means_data_set_1> <dim_data_set_1> <grid_size_data_set_1>
 *                    <num_points_data_set_2> <num_means_data_set_2> <dim_data_set_2> <grid_size_data_set_2>
 *                    <num_points_data_set_3> <num_means_data_set_3> <dim_data_set_3> <grid_size_data_set_3> 
 * ./gerador_data_set 30 2 1 30 50 10 2 50 100 25 2 100 
 */

#include <stdio.h>
#include <stdlib.h>

void gerar_arquivo(int num_points, int num_means, int dim, int grid_size, char* data_set_name)
{
	FILE* arquivo;

	arquivo = fopen(data_set_name, "wt");

	if(arquivo == NULL)
	{
		printf("Erro ao tentar abrir o arquivo!\n");
	}
	else
	{
		//escrevendo a quantidade de pontos
		fprintf(arquivo, "%s\n%d\n", "numero_de_pontos:", num_points);

		//escrevendo a quantidade de centróides
		fprintf(arquivo, "%s\n%d\n", "numero_de_centroides:", num_means);

		//escrevendo a dimensão dos pontos
		fprintf(arquivo, "%s\n%d\n", "dimensao_dos_pontos:", dim);

		//escrevendo indentificação do bloco de pontos
		fprintf(arquivo, "%s\n", "pontos:"); 

		//escrevendo pontos aleatórios 
		for(int i = 0; i < num_points; i++)
		{
			for(int j = 0; j < dim; j++)
			{
				fprintf(arquivo, "%d ", rand() % grid_size);
			}

			fprintf(arquivo, "\n");
		}

		//escrevendo indentificação do bloco de centróides
		fprintf(arquivo, "%s\n", "centróides:"); 

		//escrevendo centróides aleatórios 
		for(int i = 0; i < num_means; i++)
		{
			for(int j = 0; j < dim; j++)
			{
				fprintf(arquivo, "%d ", rand() % grid_size);
			}

			fprintf(arquivo, "\n");
		}

		fclose(arquivo);
	}
}

int main(int argc, char *argv[])
{
	int num_points_ds1 = atoll(argv[1]); //número de pontos
	int num_means_ds1 = atoll(argv[2]); //número de centróides
	int dim_ds1 = atoll(argv[3]); //dimensão dos pontos
	int grid_size_ds1 = atoll(argv[4]); //pontos variam entre [0;grid_size]
	
	   int num_points_ds2 = atoll(argv[5]);
	   int num_means_ds2 = atoll(argv[6]);
	   int dim_ds2 = atoll(argv[7]);
	   int grid_size_ds2 = atoll(argv[8]);

	   int num_points_ds3 = atoll(argv[9]);
	   int num_means_ds3 = atoll(argv[10]);
	   int dim_ds3 = atoll(argv[11]);
	   int grid_size_ds3 = atoll(argv[12]);
	 
	gerar_arquivo(num_points_ds1, num_means_ds1, dim_ds1, grid_size_ds1 , (char*) "data_set_1.txt");
	gerar_arquivo(num_points_ds2, num_means_ds2, dim_ds2, grid_size_ds2 , (char*) "data_set_2.txt");
	gerar_arquivo(num_points_ds3, num_means_ds3, dim_ds3, grid_size_ds3 , (char*) "data_set_3.txt");
}
