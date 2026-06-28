#ifndef HASH_H
#define HASH.H
#include <stdbool.h>

// estrutura de nó para tratamento de colisao por encadeamento externo
typedef struct No{
    char *nome_usuario;
    struct No *proximo;
}No;


// estrutura da Tabela Hash com variáveis para estatisticas
typedef struct{
    No **vetor_tabela;
    int tamanho;
    int total_elementos;
    int total_colisoes;
}TabelaHash;

//funções necessárias para inserir, buscar e desalocar da memória
TabelaHash* hash_criar(int tamanho);
unsigned int hash_calcular_indice(const char *texto, int tamanho_tabela);
bool hash_inserir(TabelaHash *th, const char *nome_usuario);
bool hash_buscar(TabelaHash *th, const char *nome_usuario);
void hash_destruir(TabelaHash *th); 


//funções para estatísticas
int hash_obter_total_elementos(TabelaHash *th);
int hash_obter_total_colisoes(TabelaHash *th);
float hash_obter_fator_carga(TabelaHash *th);

#endif