#include "bloom.h"
#include <stdlib.h>  // pro malloc, calloc e free
#include <string.h>  // nao precisa muito mas vai que
#include <math.h>    // pro log, exp, pow e ceil

// ----------------------------------------------------------------
// funcao hash djb2
// ----------------------------------------------------------------
// calcula um hash para a string recebida
// comeca com um numero fixo (5381) e vai misturando os caracteres
// a cada caractere faz: hash = hash * 33 + caractere
// retorna um numero inteiro que representa a string
// ----------------------------------------------------------------
static unsigned int hash_djb2(const char *str) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

// ----------------------------------------------------------------
// funcao hash sdbm
// ----------------------------------------------------------------
// outra funcao hash para calcular hash de strings
// usa shifts e somas para misturar os bits de forma diferente
// isso ajuda a ter variedade quando usamos varias funcoes hash
// ----------------------------------------------------------------
static unsigned int hash_sdbm(const char *str) {
    unsigned int hash = 0;
    int c;

    while ((c = *str++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

// ----------------------------------------------------------------
// funcao hash fnv-1a
// ----------------------------------------------------------------
// mais uma funcao hash para calcular hash de strings
// comeca com um offset e pra cada caractere faz xor e multiplicacao
// usamos essa mesma funcao na tabela hash tambem
// ----------------------------------------------------------------
static unsigned int hash_fnv1a(const char *str) {
    unsigned int hash = 2166136261u;
    int c;

    while ((c = *str++)) {
        hash ^= c;
        hash *= 16777619u;
    }

    return hash;
}

// ----------------------------------------------------------------
// setar um bit especifico no vetor
// ----------------------------------------------------------------
// o filtro de bloom guarda os dados em bits, mas em c a gente
// usa um vetor de bytes (cada byte tem 8 bits)
// entao essa funcao calcula qual byte e qual bit dentro do byte
// e seta esse bit para 1
// ----------------------------------------------------------------
static void bit_set(unsigned char *vetor, int posicao) {
    int indice_byte = posicao / 8;      // qual byte
    int posicao_bit = posicao % 8;       // qual bit dentro do byte
    vetor[indice_byte] |= (1 << posicao_bit);
}

// ----------------------------------------------------------------
// verificar se um bit ta setado
// ----------------------------------------------------------------
// mesma ideia da funcao de setar, mas ao inves de setar
// a gente verifica se o bit ja esta setado
// retorna true se estiver setado, false se nao
// ----------------------------------------------------------------
static bool bit_get(unsigned char *vetor, int posicao) {
    int indice_byte = posicao / 8;
    int posicao_bit = posicao % 8;
    return (vetor[indice_byte] & (1 << posicao_bit)) != 0;
}

// ----------------------------------------------------------------
// gerar os hashes pro filtro de bloom
// ----------------------------------------------------------------
// o filtro de bloom precisa de varias funcoes hash diferentes
// a gente tem 3 funcoes hash disponiveis, mas pode precisar de mais
// entao essa funcao usa um seed para combinar as funcoes existentes
// e gerar hashes diferentes para cada chamada
// corrigido: agora usa operacoes seguras pra evitar overflow
// ----------------------------------------------------------------
static unsigned int bloom_hash(const char *str, int seed) {
    unsigned int hash = 0;

    // escolhe qual funcao hash usar baseado no seed
    switch(seed % 3) {
        case 0:
            hash = hash_djb2(str);
            break;
        case 1:
            hash = hash_sdbm(str);
            break;
        case 2:
            hash = hash_fnv1a(str);
            break;
        default:
            hash = hash_djb2(str);
            break;
    }

    // usa operacoes seguras pra evitar overflow
    // combina o hash com o seed de forma controlada
    hash = hash ^ (seed * 0x9e3779b9);  // xor com seed multiplicado por um primo
    hash = (hash << 5) | (hash >> 27);   // rotacao circular de bits
    
    // mais uma misturada pra garantir independencia
    hash = hash ^ (hash >> 16);
    hash = hash * 0x85ebca6b;
    hash = hash ^ (hash >> 13);
    hash = hash * 0xc2b2ae35;
    hash = hash ^ (hash >> 16);

    return hash;
}

// ----------------------------------------------------------------
// criar um novo filtro de bloom
// ----------------------------------------------------------------
// aloca a memoria para o filtro e para o vetor de bits
// o vetor de bits comeca todo zerado (nenhum elemento inserido)
// retorna o ponteiro para o filtro criado
// corrigido: agora valida os parametros de entrada
// ----------------------------------------------------------------
FiltroBloom* bloom_criar(int tamanho_bits, int total_funcoes_hash) {
    // valida os parametros
    if (tamanho_bits < 8 || total_funcoes_hash < 1) {
        return NULL;
    }

    // aloca a estrutura principal
    FiltroBloom *fb = (FiltroBloom*)malloc(sizeof(FiltroBloom));
    if (fb == NULL) {
        return NULL;
    }

    // calcula quantos bytes precisa (cada byte tem 8 bits)
    int tamanho_bytes = (tamanho_bits + 7) / 8;

    // aloca o vetor de bits e zera tudo
    fb->vetor_bits = (unsigned char*)calloc(tamanho_bytes, sizeof(unsigned char));
    if (fb->vetor_bits == NULL) {
        free(fb);
        return NULL;
    }

    // guarda as configuracoes
    fb->tamanho_bits = tamanho_bits;
    fb->total_funcoes_hash = total_funcoes_hash;

    return fb;
}

// ----------------------------------------------------------------
// inserir um elemento no filtro de bloom
// ----------------------------------------------------------------
// para inserir, aplica cada funcao hash no nome do usuario
// cada hash vira uma posicao no vetor, e a gente seta esse bit
// quanto mais funcoes hash, mais bits sao setados
// corrigido: adicionada verificacao do vetor de bits
// ----------------------------------------------------------------
void bloom_inserir(FiltroBloom *fb, const char *nome_usuario) {
    if (fb == NULL || nome_usuario == NULL || fb->vetor_bits == NULL) {
        return;
    }

    // aplica cada funcao hash e seta o bit correspondente
    for (int i = 0; i < fb->total_funcoes_hash; i++) {
        unsigned int hash = bloom_hash(nome_usuario, i);
        int posicao = hash % fb->tamanho_bits;
        bit_set(fb->vetor_bits, posicao);
    }
}

// ----------------------------------------------------------------
// buscar um elemento no filtro de bloom
// ----------------------------------------------------------------
// para buscar, aplica as mesmas funcoes hash da insercao
// se todos os bits estiverem setados, retorna true (possivelmente existe)
// se algum bit nao estiver setado, retorna false (definitivamente nao existe)
// corrigido: adicionada verificacao do vetor de bits
// ----------------------------------------------------------------
bool bloom_buscar(FiltroBloom *fb, const char *nome_usuario) {
    if (fb == NULL || nome_usuario == NULL || fb->vetor_bits == NULL) {
        return false;
    }

    // verifica todos os bits correspondentes
    for (int i = 0; i < fb->total_funcoes_hash; i++) {
        unsigned int hash = bloom_hash(nome_usuario, i);
        int posicao = hash % fb->tamanho_bits;

        // se algum bit nao estiver setado, nao existe
        if (!bit_get(fb->vetor_bits, posicao)) {
            return false;
        }
    }

    // todos os bits estao setados, entao "possivelmente existe"
    return true;
}

// ----------------------------------------------------------------
// recomendar dimensoes do filtro de bloom
// ----------------------------------------------------------------
// ajuda a escolher o tamanho do vetor e o numero de funcoes hash
// baseado no numero de elementos e na taxa de falso positivo desejada
// usa formulas matematicas pra calcular os valores ideais
// corrigido: tratamento de overflow e limites mais realistas
// ----------------------------------------------------------------
void bloom_recomendar_dimensoes(int num_elementos, double taxa_falso_positivo, int *tamanho_bits, int *num_funcoes_hash) {
    // valida os parametros
    if (num_elementos <= 0 || taxa_falso_positivo <= 0 || taxa_falso_positivo >= 1) {
        *tamanho_bits = 1024;
        *num_funcoes_hash = 3;
        return;
    }

    double log2 = 0.6931471805599453;

    // calcula o tamanho ideal do vetor
    // cuidado com overflow: faz a conta de forma segura
    double ln_p = log(taxa_falso_positivo);
    double numerador = -(double)num_elementos * ln_p;
    double denominador = log2 * log2;
    *tamanho_bits = (int)ceil(numerador / denominador);

    // calcula o numero ideal de funcoes hash
    *num_funcoes_hash = (int)ceil(((*tamanho_bits) / (double)num_elementos) * log2);

    // garante valores minimos
    if (*tamanho_bits < 64) *tamanho_bits = 64;
    if (*num_funcoes_hash < 1) *num_funcoes_hash = 1;
    
    // limita o numero de funcoes hash baseado no tamanho do vetor
    // nao faz sentido ter mais hashs que bits
    if (*num_funcoes_hash > *tamanho_bits) {
        *num_funcoes_hash = *tamanho_bits;
    }
}

// ----------------------------------------------------------------
// calcular a probabilidade teorica de falso positivo
// ----------------------------------------------------------------
// calcula qual a taxa de falso positivo esperada para o filtro
// usando a formula: (1 - e^(-k*n/m))^k
// isso ajuda a ver se a configuracao escolhida esta boa
// ----------------------------------------------------------------
double bloom_calcular_falso_positivo(int tamanho_bits, int num_funcoes_hash, int num_elementos) {
    // valida os parametros
    if (tamanho_bits <= 0 || num_funcoes_hash <= 0 || num_elementos <= 0) {
        return 1.0; // probabilidade maxima
    }

    double m = tamanho_bits;
    double n = num_elementos;
    double k = num_funcoes_hash;

    // evita divisao por zero e overflow
    if (m <= 0) return 1.0;
    
    double expoente = -k * n / m;
    // protege contra valores extremos
    if (expoente < -50.0) expoente = -50.0; // evita underflow
    if (expoente > 50.0) expoente = 50.0;   // evita overflow
    
    double prob = pow(1 - exp(expoente), k);
    
    // garante que a probabilidade fique entre 0 e 1
    if (prob < 0) prob = 0;
    if (prob > 1) prob = 1;
    
    return prob;
}

// ----------------------------------------------------------------
// calcular a taxa de ocupacao do vetor
// ----------------------------------------------------------------
// conta quantos bits estao setados e calcula a porcentagem
// se a taxa de ocupacao estiver muito alta, o filtro vai ter
// muitos falsos positivos porque muitos bits estao setados
// corrigido: agora conta apenas os bits validos
// ----------------------------------------------------------------
double bloom_taxa_ocupacao(FiltroBloom *fb) {
    if (fb == NULL || fb->vetor_bits == NULL) {
        return 0.0;
    }

    int bits_ocupados = 0;
    int total_bits = fb->tamanho_bits;
    int bytes_total = (total_bits + 7) / 8;

    // conta os bits setados, mas so ate o total_bits
    for (int i = 0; i < bytes_total; i++) {
        unsigned char byte = fb->vetor_bits[i];
        // calcula quantos bits desse byte sao validos
        int bits_no_byte = (i == bytes_total - 1) ? (total_bits % 8) : 8;
        if (bits_no_byte == 0) bits_no_byte = 8; // ultimo byte completo
        
        // conta apenas os bits validos
        for (int j = 0; j < bits_no_byte; j++) {
            if (byte & (1 << j)) {
                bits_ocupados++;
            }
        }
    }

    return (double)bits_ocupados / total_bits;
}

// ----------------------------------------------------------------
// destruir o filtro de bloom e liberar memoria
// ----------------------------------------------------------------
// libera primeiro o vetor de bits, depois a estrutura principal
// sempre importante liberar a memoria alocada para nao vazar
// ----------------------------------------------------------------
void bloom_destruir(FiltroBloom *fb) {
    if (fb == NULL) {
        return;
    }

    if (fb->vetor_bits != NULL) {
        free(fb->vetor_bits);
        fb->vetor_bits = NULL;
    }

    free(fb);
}
