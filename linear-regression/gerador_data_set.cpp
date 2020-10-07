/**
 * PUCRS - FACIN - GMAP
 *
 * Modificações no código: (Gabriell Araujo)
 * Versão: (04/03/2016)
 *
 * Este programa gera os data_sets para execução do algoritmo de regressão linear.
 *
 * Comando de compilação:
 * g++ -O3 gerador_data_set.cpp -o gerador_data_set
 *
 * Comando de execução:
 * ./gerador_data_set <n_pontos_data_set_1> <n_pontos_data_set_2> <n_pontos_data_set_3> <intervalo_valores>
 */
 
 /**
 * sugestão de tamanho dos data_sets:
 * data_set_1 = 9000000 points _ 50,4 mb
 * data_set_2 = 18000000 points _ 100,8 mb
 * data_set_3 = 90000000 points _ 504,0 mb
 *
 * sugestão de intervalo de valores: 50
 *
 * sugestão de execução:
 * ./gerador_data_set 9000000 18000000 90000000 50
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
        fprintf(arquivo, "%lld\n", quantidadeDePontos);
	
        for(long long i = 0; i < quantidadeDePontos; i++)
        {
		fprintf(arquivo, "%lld %lld\n", rand() % intervalo_valores, rand() % intervalo_valores);
	}

	fclose(arquivo);
    }
}

int main(int argc, char *argv[])
{
	long long int n_pontos_data_set_1 = atoll(argv[1]);
	long long int n_pontos_data_set_2 = atoll(argv[2]);
	long long int n_pontos_data_set_3 = atoll(argv[3]);
	intervalo_valores = atoll(argv[4]);
	
	gerarArquivoDeTeste(n_pontos_data_set_1, (char*) "lr_data_set_1.txt");
	gerarArquivoDeTeste(n_pontos_data_set_2, (char*) "lr_data_set_2.txt");
	gerarArquivoDeTeste(n_pontos_data_set_3, (char*) "lr_data_set_3.txt");
}
