#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

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

    // Essas funções serão implementadas depois
    // hash_inserir(hash, usuario);
    // bloom_inserir(bloom, usuario);

    printf("\nUsuario cadastrado com sucesso!\n");
}



void carregar_arquivo() { 

    FILE* arquivo = fopen("usuarios.txt", "r");

    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    fclose(arquivo);
}



int main() {

    setlocale(LC_ALL, "");

    int opcao;
    int continuar = 1;

    while (continuar) {

        menu();

        scanf("%d", &opcao);
        getchar();

        switch(opcao) {

        case 1:
            break;

        case 2:
            break;

        case 3:
            printf("\nEstatisticas.\n");
            break;

        case 4:
            printf("\nInserir usuarios em lote.\n");
            break;

        case 5:
            continuar = 0;
            break;

        default:
            printf("Opcao invalida!\n");
        }
        
        
    }

    printf("\nPrograma finalizado!\n");

    return 0;
}