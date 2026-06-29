#include <stdio.h>

#define TOTAL_LETRAS 8

void gerar_usuario(int numero, char usuario[13]) {

    int valor = numero;

    // Gera as 8 letras
    for (int i = TOTAL_LETRAS - 1; i >= 0; i--) {
        usuario[i] = 'a' + (valor % 26);
        valor /= 26;
    }

    // Gera os 3 números finais
    sprintf(usuario + 8, "%03d", numero % 1000);
}

void gerar_arquivo(const char *nome_arquivo, int quantidade) {

    FILE *arquivo = fopen(nome_arquivo, "w");

    if (arquivo == NULL) {
        printf("Erro ao criar %s\n", nome_arquivo);
        return;
    }

    char usuario[13];

    for (int i = 0; i < quantidade; i++) {

        gerar_usuario(i, usuario);

        fprintf(arquivo, "%s\n", usuario);
    }

    fclose(arquivo);

    printf("%s criado com %d usuarios.\n", nome_arquivo, quantidade);
}

int main() {

    gerar_arquivo("usuarios_1000.txt", 1000);

    gerar_arquivo("usuarios_10000.txt", 10000);

    gerar_arquivo("usuarios_100000.txt", 100000);

    printf("\nArquivos gerados com sucesso!\n");

    return 0;
}