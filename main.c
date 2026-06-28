#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

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

    // Essas funções serão implementadas depois
    // hash_inserir(hash, usuario);
    // bloom_inserir(bloom, usuario);

    printf("\nUsuario cadastrado com sucesso!\n");
}


void consultar_usuario() {

    char usuario[12];

    printf("---------------------\n");
    printf("|  CONSULTAR USUARIO |\n");
    printf("---------------------\n");

    printf("Usuario (8 letras + 3 numeros) ");
    scanf("%11s", usuario);

    getchar();

 // if (!bloom_consultar(bloom, usuario)) {
    //     printf("\n Usuario nao encontrado!\n");
    //     return;
    // }

    //se o filtro de bloom retornar que o usuario pode existir consultamos a tabela hash

    // Usuario *resultado = hash_consultar(hash, usuario);

    // if (resultado != NULL) {
    //     printf("\nUsuario encontrado!\n");
    //     printf("Usuario: %s\n", resultado->usuario);
    //     printf("Nome: %s\n", resultado->nome);

    // } else {
    //     printf("\nUsuario nao encontrado.\n");
    //     printf("Falso positivo do Filtro de Bloom.\n");
    // falsos_positivos++;
    // }
}


void Mostrar_estatisticas() { 
    printf("\n--- ESTATÍSTICAS DA TABELA HASH ---\n");
    // hash_exibir_estatisticas(hash);
    printf("\n----ESTATÍSTICAS DO FILTRO BLOOM  ---\n");
    // bloom_exibir_estatisticas(bloom);
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
        inserir_usuario();
            break;
            
            case 2:
            printf("\nConsultar usuarios.\n");
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