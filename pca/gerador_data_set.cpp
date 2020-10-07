/**
 * PUCRS - FACIN - GMAP
 *
 * Modificações no código: (Gabriell Araujo)
 * Versão: (04/03/2016)
 *
 * Este programa gera os arquivos de dados para execução do algoritmo pca.
 *
 * Comando de compilação:
 * g++ -O3 gerador_data_set.cpp -o gerador_data_set
 *
 * Comando de execução:
 * ./gerador_data_set <n_pontos_data_1> <n_pontos_data_2> <n_pontos_data_3> <intervalo_valores>
 */
 
 /**
 * sugestão de parâmetros dos arquivos:
 * data_1 = 500 x 500
 * data_2 = 1000 x 1000
 * data_3 = 1500 x 1500
 *
 * sugestão de intervalo de valores: 50
 *
 * sugestão de execução:
 * ./gerador_data_set 500 1000 1500 50
 */ 

#include <stdio.h>
#include <stdlib.h>

long long int intervalo_valores;

void gerarArquivoDeTeste(long long int quantidadeDePontos, char* nomeDoArquivo)
{
    FILE* arquivo;

    arquivo = fopen(nomeDoArquivo, "wt");

    if(arquivo == NULL)
    {
        printf("Erro ao tentar abrir o arquivo!\n");
    }
    else
    {
        fprintf(arquivo, "%lld %lld\n", quantidadeDePontos, quantidadeDePontos);

		for (int i=0; i < quantidadeDePontos; i++) 
		{
			for (int j=0; j < quantidadeDePontos; j++) 
			{
				fprintf(arquivo, "%lld ", rand() % intervalo_valores);
			}
	
			fprintf(arquivo, "\n");
		}

		fclose(arquivo);		
	}
	
}

int main(int argc, char *argv[])
{
	long long int n_pontos_data_1 = atoll(argv[1]);
	long long int n_pontos_data_2 = atoll(argv[2]);
	long long int n_pontos_data_3 = atoll(argv[3]);
	intervalo_valores = atoll(argv[4]);
	
	gerarArquivoDeTeste(n_pontos_data_1, (char*) "pca_data_1.txt");
	gerarArquivoDeTeste(n_pontos_data_2, (char*) "pca_data_2.txt");
	gerarArquivoDeTeste(n_pontos_data_3, (char*) "pca_data_3.txt");
}
