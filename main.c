#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#include "hash.h"
#include "bloom.h"
#define TAMANHO_HASH 12007
#define TAMANHO_BLOOM 65536
#define NUM_FUNCOES_BLOOM 7

TabelaHash *hash = NULL;
FiltroBloom *bloom = NULL;

int falsos_positivos = 0;

void menu() {

    printf("\n");
    printf("------------------------------\n");
    printf("|      ESCOLHA UMA OPCAO      |\n");
    printf("------------------------------\n");
    printf("[1] Cadastrar novo usuario\n");
    printf("[2] Consultar usuarios\n");
    printf("[3] Estatisticas\n");
    printf("[4] Inserir usuarios em lote\n");
    printf("[5] Sair\n");
    printf("Opcao: ");
}

void inserir_usuario() {

    char usuario[12];

    printf("\n---------------------------\n");
    printf("| INSERIR NOVO USUARIO     |\n");
    printf("---------------------------\n");

    printf("Usuario (8 letras + 3 numeros) ");
    scanf("%11s", usuario);

    getchar();

    
    if (hash_inserir(hash, usuario)) {
    bloom_inserir(bloom, usuario);
    printf("\nUsuario cadastrado com sucesso!\n");
} else {
    printf("\nUsuario ja cadastrado!\n");
}
}


void consultar_usuario() {

    char usuario[12];

    printf("---------------------\n");
    printf("|  CONSULTAR USUARIO |\n");
    printf("---------------------\n");

    printf("Usuario (8 letras + 3 numeros) ");
    scanf("%11s", usuario);

    getchar();

if (!bloom_buscar(bloom, usuario)) {
    printf("\nUsuario nao encontrado!\n");
    return;
}

    //se o filtro de bloom retornar que o usuario pode existir consultamos a tabela hash

   if (hash_buscar(hash, usuario)) {

    printf("\nUsuario encontrado!\n");
    printf("Usuario: %s\n", usuario);

} else {

    printf("\nUsuario nao encontrado.\n");
    printf("Falso positivo do Filtro de Bloom.\n");
    falsos_positivos++;
}
}

void Mostrar_estatisticas() {

    printf("\n--- ESTATISTICAS DA TABELA HASH ---\n");

    printf("Total de elementos: %d\n",
           hash_obter_total_elementos(hash));

    printf("Total de colisoes: %d\n",
           hash_obter_total_colisoes(hash));

    printf("Fator de carga: %.2f\n",
           hash_obter_fator_carga(hash));

    printf("\n--- ESTATISTICAS DO BLOOM FILTER ---\n");

    printf("Taxa de ocupacao: %.2lf%%\n",
           bloom_taxa_ocupacao(bloom) * 100);

    printf("Falsos positivos encontrados: %d\n",
           falsos_positivos);

    printf("Probabilidade teorica: %.6lf\n",
           bloom_calcular_falso_positivo(
               bloom->tamanho_bits,
               bloom->total_funcoes_hash,
               hash_obter_total_elementos(hash)
           ));
}





void carregar_arquivo() {

    
    int opcao;
    char nome_arquivo[30];

    printf("\nQual arquivo deseja carregar?\n");
    printf("[1] usuarios_1000.txt\n");
    printf("[2] usuarios_10000.txt\n");
    printf("[3] usuarios_100000.txt\n");
    printf("Opcao: ");
    scanf("%d", &opcao);
    getchar();

    switch (opcao) {

        case 1:
            strcpy(nome_arquivo, "usuarios_1000.txt");
            break;

        case 2:
            strcpy(nome_arquivo, "usuarios_10000.txt");
            break;

        case 3:
            strcpy(nome_arquivo, "usuarios_100000.txt");
            break;

        default:
            printf("Opcao invalida!\n");
            return;
    }

    FILE *arquivo = fopen(nome_arquivo, "r");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s\n", nome_arquivo);
        return;
    }

    char usuario[12];
    int inseridos = 0;

    while (fscanf(arquivo, "%11s", usuario) == 1) {

        if (hash_inserir(hash, usuario)) {

            bloom_inserir(bloom, usuario);
            inseridos++;
        }
    }

    fclose(arquivo);

    printf("\n%d usuarios inseridos do arquivo %s\n", inseridos, nome_arquivo);
}


int main() {

    setlocale(LC_ALL, "");

   hash = hash_criar(TAMANHO_HASH);
   bloom = bloom_criar(TAMANHO_BLOOM, NUM_FUNCOES_BLOOM);

    if (hash == NULL || bloom == NULL) {
    printf("Erro ao criar as estruturas de dados.\n");
    return 1;
}

    int opcao;
    int continuar = 1;

    while (continuar) {

        menu();

        scanf("%d", &opcao);
        getchar();

        switch(opcao) {

        case 1: 
            inserir_usuario();
            break;
            
            case 2:
            consultar_usuario();
            break;

        case 3:
            Mostrar_estatisticas();
            break;

        case 4:
               carregar_arquivo();
            break;

        case 5: 
            continuar = 0;
            break;
            
            default:
            printf("Opcao invalida!\n");
        }
        
        
    }
    printf("\nPrograma finalizado!\n");



    hash_destruir(hash);
    bloom_destruir(bloom);

    return 0;
}