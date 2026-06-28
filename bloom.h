#ifndef BLOOM_H
#define BLOOM_H

#include <stdbool.h>

//definição da estrutura principal do filtro de bloom

typedef struct {
    unsigned char *vetor_bits;
    int tamanho_bits;
    int total_funcoes_hash;
}FiltroBloom;

//funções necessarias para inserir, buscar e desalocar a memória
FiltroBloom* bloom_criar(int tamanho_bits, int total_funcoes_hash);
void bloom_inserir(FiltroBloom *fb, const char *nome_usuario);
bool bloom_buscar(FiltroBloom *fb, const char *nome_usuario);
void bloom_destruir(FiltroBloom *fb);

#endif