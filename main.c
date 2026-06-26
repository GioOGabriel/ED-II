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
            break;

        case 4:
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