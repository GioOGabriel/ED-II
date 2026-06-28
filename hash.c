#include "hash.h"
#include <stdlib.h>  // pro malloc e free
#include <string.h>  // pro strdup e strcmp

// ----------------------------------------------------------------
// funcao que calcula o indice na tabela - fnv-1a
// ----------------------------------------------------------------
// escolhi o fnv-1a porque eh rapidinho e espalha bem as strings.
// ele pega cada caractere, faz xor com o valor atual e multiplica
// por um primo grandao (0x01000193). isso faz com que strings
// parecidas caiam em indices bem diferentes.
//
// o esquema:
// 1) comeca com 2166136261u (um offset fixo)
// 2) pra cada letra: xor com o hash, depois multiplica pelo primo
// 3) no final usa % pra caber no tamanho da tabela
// ----------------------------------------------------------------
unsigned int hash_calcular_indice(const char *texto, int tamanho_tabela) {
    // constantes padrao do fnv-1a
    const unsigned int fnv_offset = 2166136261u;
    const unsigned int fnv_prime = 0x01000193;

    unsigned int hash = fnv_offset;

    // vai letra por letra ate o \0
    while (*texto != '\0') {
        hash ^= (unsigned char)(*texto); // xor com a letra
        hash *= fnv_prime;               // multiplica pelo primo
        texto++;                          // proxima letra
    }

    // reduz pro tamanho da tabela
    return hash % tamanho_tabela;
}

// ----------------------------------------------------------------
// criar tabela hash
// ----------------------------------------------------------------
// aloca a struct principal e o vetor de ponteiros.
// usa calloc porque ja deixa tudo null (menos trabalho).
// ----------------------------------------------------------------
TabelaHash* hash_criar(int tamanho) {

     if (tamanho <= 0) {
        return NULL;
     }
     
    // aloca a tabela em si
    TabelaHash *th = (TabelaHash*) malloc(sizeof(TabelaHash));
    if (th == NULL) {
        return NULL;
    }

    th->tamanho = tamanho;
    th->total_elementos = 0;
    th->total_colisoes = 0;

    // calloc ja zera a memoria (tudo null)
    th->vetor_tabela = (No**) calloc((size_t)tamanho, sizeof(No*));
    if (th->vetor_tabela == NULL) {
        free(th);
        return NULL;
    }

    return th;
}

// ----------------------------------------------------------------
// inserir um nome de usuario na tabela
// ----------------------------------------------------------------
// calcula o indice, ve se ja existe pra nao duplicar, e insere
// no comeco da lista. se ja tiver alguem na posicao, conta como
// colisao.
// retorna true se conseguiu inserir, false se ja existia.
// ----------------------------------------------------------------
bool hash_inserir(TabelaHash *th, const char *nome_usuario) {
    // descobre onde vai colocar
    unsigned int indice = hash_calcular_indice(nome_usuario, th->tamanho);

    // ve se ja tem esse usuario na lista (nada de duplicata)
    No *atual = th->vetor_tabela[indice];
    while (atual != NULL) {
        if (strcmp(atual->nome_usuario, nome_usuario) == 0) {
            return false; // ja existe, nao insere
        }
        atual = atual->proximo;
    }

    // se ja tem alguem nessa posicao, eh colisao
    if (th->vetor_tabela[indice] != NULL) {
        th->total_colisoes++;
    }

    // aloca espaco pro novo no
    No *novo_no = (No*) malloc(sizeof(No));
    if (novo_no == NULL) {
        return false;
    }

    // faz uma copia da string (strdup aloca e copia)
    novo_no->nome_usuario = strdup(nome_usuario);
    if (novo_no->nome_usuario == NULL) {
        free(novo_no);
        return false;
    }

    // insere no comeco da lista
    novo_no->proximo = th->vetor_tabela[indice];
    th->vetor_tabela[indice] = novo_no;

    th->total_elementos++;
    return true;
}

// ----------------------------------------------------------------
// buscar um nome de usuario na tabela
// ----------------------------------------------------------------
// calcula o indice, percorre a lista ate achar.
// retorna true se achou, false se nao.
// ----------------------------------------------------------------
bool hash_buscar(TabelaHash *th, const char *nome_usuario) {
    unsigned int indice = hash_calcular_indice(nome_usuario, th->tamanho);

    No *atual = th->vetor_tabela[indice];

    // vai andando pela lista ate achar a string ou chegar no fim
    while (atual != NULL) {
        if (strcmp(atual->nome_usuario, nome_usuario) == 0) {
            return true; // achou
        }
        atual = atual->proximo;
    }

    return false; // nao tem
}

// ----------------------------------------------------------------
// remover um nome de usuario da tabela
// ----------------------------------------------------------------
// calcula o indice, percorre a lista ate achar e remove.
// ajusta os ponteiros, libera a string e o no.
// retorna true se removeu, false se nao encontrou.
// ----------------------------------------------------------------
bool hash_remover(TabelaHash *th, const char *nome_usuario) {
    unsigned int indice = hash_calcular_indice(nome_usuario, th->tamanho);

    No *atual = th->vetor_tabela[indice];
    No *anterior = NULL;

    while (atual != NULL) {
        if (strcmp(atual->nome_usuario, nome_usuario) == 0) {
            // achou, agora remove

            if (anterior == NULL) {
                // eh o primeiro da lista
                th->vetor_tabela[indice] = atual->proximo;
            } else {
                // ta no meio ou no fim
                anterior->proximo = atual->proximo;
            }

            free(atual->nome_usuario);
            free(atual);
            th->total_elementos--;
            return true;
        }

        anterior = atual;
        atual = atual->proximo;
    }

    return false; // nao encontrou
}

// ----------------------------------------------------------------
// destruir a tabela e liberar memoria
// ----------------------------------------------------------------
// precisa liberar cada no, cada string, o vetor e a struct.
// se esquecer, vaza memoria e o professor reclama.
// ----------------------------------------------------------------
void hash_destruir(TabelaHash *th) {
    if (th == NULL) {
        return;
    }

    // percorre cada posicao do vetor
    for (int i = 0; i < th->tamanho; i++) {
        No *atual = th->vetor_tabela[i];

        // vai liberando no por no
        while (atual != NULL) {
            No *temp = atual;            // guarda o atual
            atual = atual->proximo;      // vai pro proximo
            free(temp->nome_usuario);    // libera a string
            free(temp);                  // libera o no
        }
    }

    free(th->vetor_tabela); // libera o vetor
    free(th);               // libera a struct
}

// ----------------------------------------------------------------
// funcoes de estatistica
// ----------------------------------------------------------------

// retorna quantos elementos tem na tabela
int hash_obter_total_elementos(TabelaHash *th) {
    return th->total_elementos;
}

// retorna quantas colisoes aconteceram
int hash_obter_total_colisoes(TabelaHash *th) {
    return th->total_colisoes;
}

// calcula o fator de carga (elementos / tamanho)
float hash_obter_fator_carga(TabelaHash *th) {
    if (th->tamanho == 0) {
        return 0.0f;
    }
    return (float) th->total_elementos / (float) th->tamanho;
}
