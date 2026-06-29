#define _DEFAULT_SOURCE  


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#include "hash.h"
#include "bloom.h"
#define TAMANHO_HASH 12007 // primo, foi escolhido para reduzir colisoes na tabela hash
#define TAMANHO_BLOOM 65536 // bloom global do menu — adequado para uso manual
#define NUM_FUNCOES_BLOOM 7 // k ideal, calculado pela formula otima

// estrtuturas globais
TabelaHash *hash = NULL;
FiltroBloom *bloom = NULL;

// contadores das consultas manuais
int falsos_positivos = 0;
int consultas_realizadas = 0;
int consultas_evitas_bloom = 0;
double tempo_total_consultas = 0.0;

// guarda o resultado do ultimo experimento automatico (opcao 5 do menu) -1 indica que o experimento ainda nao foi executado                    
int exp_ultimo_falsos_pos = -1;
double exp_ultimo_taxa_fp = -1.0;
int exp_ultimo_n = 0;


// gera nomes no formato 8letras+3numeros para testar falsos positivos 
void gerar_nome_aleatorio(char *buffer) {
    static const char letras[] = "abcdefghijklmnopqrstuvwxyz";
    static const char numeros[] = "0123456789";

    for (int i = 0; i < 8; i++) {
        buffer[i] = letras[rand() % 26];
    }
    for (int i = 0; i < 3; i++) {
        buffer[8 + i] = numeros[rand() % 10];
    }
    buffer[11] = '\0';
}

// executa os experimentos automaticos, aqueles que foram pedidos no trabalho
//com 1000, 10000 e 100000 registros, e mede o tempo de busca com e sem bloom, alem de calcular a taxa de falsos positivos

void executar_experimentos() {

    const char *arquivos[] = {
        "data/usuarios_1000.txt",
        "data/usuarios_10000.txt",
        "data/usuarios_100000.txt"
    };
    const int quantidades[] = {1000, 10000, 100000};
    const int NUM_CENARIOS = 3;

    printf("\n---------------------------------------------------------------------\n");
    printf("  | EXPERIMENTO: TEMPO SEM BLOOM x TEMPO COM BLOOM x FALSOS POSITIVOS |\n");
    printf("  ---------------------------------------------------------------------\n");
    printf("%-12s %-20s %-20s %-15s %-12s %-10s\n",
        "Quantidade", "Tempo sem Bloom(s)", "Tempo com Bloom(s)",
        "Falsos Pos(%)", "Bits usados", "Num funcs");

    FILE *csv = fopen("resultados_experimentos.csv", "w");
    if (csv != NULL) {
        fprintf(csv, "quantidade,tempo_sem_bloom,tempo_com_bloom,falsos_positivos,taxa_falsos_positivos,bits_usados,num_funcoes\n");
    }

    for (int idx = 0; idx < NUM_CENARIOS; idx++) {

        int n = quantidades[idx];

        // calcula m e k ideais para ~1% de falso positivo nesse cenario 
        int bits_recomendados, funcs_recomendadas;
        bloom_recomendar_dimensoes(n, 0.01, &bits_recomendados, &funcs_recomendadas);

        // cria estruturas locais pra nao interferir com o bloom global do menu
        TabelaHash *th_teste = hash_criar(TAMANHO_HASH);
        FiltroBloom *fb_teste = bloom_criar(bits_recomendados, funcs_recomendadas);

        if (th_teste == NULL || fb_teste == NULL) {
            printf("Erro ao criar estruturas para o cenario de %d registros.\n", n);
            continue;
        } 
       //abre o arquivo de usuarios do cenario atual
        FILE *arq = fopen(arquivos[idx], "r");
        if (arq == NULL) {
            printf("Erro ao abrir %s (verifique se o arquivo esta na mesma pasta do executavel).\n",
                arquivos[idx]);
            hash_destruir(th_teste);
            bloom_destruir(fb_teste);
            continue;
        }

        // le todos os usuarios e insere na hash e no bloom
        char **nomes = (char **) malloc((size_t) n * sizeof(char *));
        int total_lidos = 0;
        char buffer[12];

        while (total_lidos < n && fscanf(arq, "%11s", buffer) == 1) {
            nomes[total_lidos] = strdup(buffer);
            hash_inserir(th_teste, buffer);
            bloom_inserir(fb_teste, buffer);
            total_lidos++;
        }
        fclose(arq);

        // busca direto na hash, sem usar o bloom, e mede o tempo
        clock_t inicio_sem = clock();
        for (int i = 0; i < total_lidos; i++) {
            hash_buscar(th_teste, nomes[i]);
        }
        clock_t fim_sem = clock();
        double tempo_sem_bloom = (double) (fim_sem - inicio_sem) / CLOCKS_PER_SEC;

       // Busca usando o bloom primeiro, e se o bloom disser que existe, busca na hash, e mede o tempo
        clock_t inicio_com = clock();
        for (int i = 0; i < total_lidos; i++) {
            if (bloom_buscar(fb_teste, nomes[i])) {
                hash_buscar(th_teste, nomes[i]);
            }
        }
        clock_t fim_com = clock();
        double tempo_com_bloom = (double) (fim_com - inicio_com) / CLOCKS_PER_SEC;

        // testa falsos positivos: gera nomes aleatorios que nao estao na hash, e ve quantos o bloom diz que existem
        int falsos_pos = 0;

        srand((unsigned int) time(NULL) + (unsigned int) idx);

        for (int i = 0; i < total_lidos; i++) {
            char nome_falso[12];
            int tentativas = 0;

            // gera nomes ate encontrar um que definitivamente nao esta cadastrado
            do {
                gerar_nome_aleatorio(nome_falso);
                tentativas++;
            } while (hash_buscar(th_teste, nome_falso) && tentativas < 100);

            // se o bloom disser que existe, e um falso positivo — o nome nao esta la
            if (bloom_buscar(fb_teste, nome_falso)) {
                falsos_pos++;
            }
        }

        // calcula a taxa percentual de falsos positivos
        double taxa_fp = (total_lidos > 0)
            ? ((double) falsos_pos / (double) total_lidos) * 100.0
            : 0.0;

        /* imprime a linha da tabela com todas as colunas */
        printf("%-12d %-20.6f %-20.6f %-15.2f %-12d %-10d\n",
            total_lidos, tempo_sem_bloom, tempo_com_bloom, taxa_fp,
            bits_recomendados, funcs_recomendadas);

        if (csv != NULL) {
            fprintf(csv, "%d,%.6f,%.6f,%d,%.2f,%d,%d\n",
                    total_lidos, tempo_sem_bloom, tempo_com_bloom,
                    falsos_pos, taxa_fp, bits_recomendados, funcs_recomendadas);
        }

        //atualiza as variaveis globais com o resultado deste cenario
        //ao final do loop ficam os dados do maior cenario (100k)   
        exp_ultimo_falsos_pos = falsos_pos;
        exp_ultimo_taxa_fp    = taxa_fp;
        exp_ultimo_n          = total_lidos;

        //libera a memoria deste cenario antes de passar para o proximo
        for (int i = 0; i < total_lidos; i++) {
            free(nomes[i]);
        }
        free(nomes);
        hash_destruir(th_teste);
        bloom_destruir(fb_teste);
    }

    if (csv != NULL) {
        fclose(csv);
        printf("\nResultados salvos em 'resultados_experimentos.csv'\n");
    }
}

void menu() {

    printf("\n");
    printf("------------------------------\n");
    printf("|      ESCOLHA UMA OPCAO      |\n");
    printf("------------------------------\n");
    printf("[1] Cadastrar novo usuario\n");
    printf("[2] Consultar usuarios\n");
    printf("[3] Estatisticas\n");
    printf("[4] Inserir usuarios em lote\n");
    printf("[5] Executar experimentos (1000/10000/100000)\n");
    printf("[6] Sair\n");
    printf("Opcao: ");
}


// insere um novo usuario na hash e no bloom, se ainda nao estiver cadastrado
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

// consulta um usuario: primeiro verifica no bloom, se o bloom disser que existe, busca na hash
void consultar_usuario() {

    char usuario[12];

    printf("---------------------\n");
    printf("|  CONSULTAR USUARIO |\n");
    printf("---------------------\n");

    printf("Usuario (8 letras + 3 numeros) ");
    scanf("%11s", usuario);

    getchar();

    // Início da medição do tempo
    clock_t inicio = clock();

    if (!bloom_buscar(bloom, usuario)) {

        consultas_evitas_bloom++;

        printf("\nUsuario nao encontrado!\n");

        // Fim da medição
        clock_t fim = clock();

        double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

        tempo_total_consultas += tempo;
        consultas_realizadas++;

        return;
    }

    // Consulta na tabela hash
    if (hash_buscar(hash, usuario)) {

        printf("\nUsuario encontrado!\n");
        printf("Usuario: %s\n", usuario);

    } else {

        printf("\nUsuario nao encontrado.\n");
        printf("Falso positivo do Filtro de Bloom.\n");

        falsos_positivos++;
    }

    // Fim da medição
    clock_t fim = clock();

    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    tempo_total_consultas += tempo;
    consultas_realizadas++;
}



// essa funcao exibe as estatisticas do sistema
// mostra dados da tabela hash, do filtro de bloom e das consultas manuais realizadas pelo usuario via opcao 2 do menu
// tambem exibe o resultado do ultimo experimento automatico, se ja tiver sido executado via opcao 5

void Mostrar_estatisticas() {
 
    double tempo_medio = 0.0;
    double taxa_fp = 0.0;
 
    //consultas que chegaram ate a hash (passaram pelo bloom) 
    int consultas_hash = consultas_realizadas - consultas_evitas_bloom;
 
    if (consultas_realizadas > 0) {
        tempo_medio = tempo_total_consultas / consultas_realizadas;
    }
 
    if (consultas_hash > 0) {
        taxa_fp = (double)falsos_positivos / consultas_hash * 100.0;
    }
 
    printf("\n--- ESTATISTICAS DA TABELA HASH ---\n");
    printf("Total de elementos: %d\n",
        hash_obter_total_elementos(hash));
    printf("Total de colisoes: %d\n",
        hash_obter_total_colisoes(hash));
    printf("Fator de carga: %.2f\n",
        hash_obter_fator_carga(hash));
 
    printf("\n--- ESTATISTICAS DO BLOOM FILTER ---\n");
    printf("Tamanho do vetor (bits): %d\n",
        bloom->tamanho_bits);
    printf("Numero de funcoes hash: %d\n",
        bloom->total_funcoes_hash);
    printf("Taxa de ocupacao: %.2lf%%\n",
           bloom_taxa_ocupacao(bloom) * 100);
    printf("Falsos positivos encontrados: %d\n",
        falsos_positivos);
    printf("Probabilidade teorica de falso positivo: %.6lf\n",
        bloom_calcular_falso_positivo(
            bloom->tamanho_bits,
            bloom->total_funcoes_hash,
            hash_obter_total_elementos(hash)
        ));

    printf("\n--- ESTATISTICAS DAS CONSULTAS MANUAIS ---\n");
    printf("Consultas realizadas: %d\n", consultas_realizadas);
    printf("Consultas evitadas pelo Bloom: %d\n", consultas_evitas_bloom);
if (consultas_realizadas > 0) {
    printf("Percentual evitado pelo Bloom: %.2f%%\n",
        (double)consultas_evitas_bloom / consultas_realizadas * 100.0);
}
    printf("Taxa de falsos positivos: %.2lf%%\n",taxa_fp);
    printf("Tempo medio de consulta: %.9lf segundos\n",tempo_medio);

    // Por fim, exibe resultado do ultimo experimento automatico, se disponivel
    printf("\n--- RESULTADO DO ULTIMO EXPERIMENTO AUTOMATICO ---\n");
    if (exp_ultimo_falsos_pos >= 0) {
        printf("Cenario: %d registros\n", exp_ultimo_n);
        printf("Falsos positivos: %d\n", exp_ultimo_falsos_pos);
        printf("Taxa de falsos positivos: %.2f%%\n", exp_ultimo_taxa_fp);
    } else {
        printf("Experimento automatico ainda nao executado (use a opcao 5).\n");
    }
}


// essa funcao carrega um arquivo de usuarios (opcao 4 do menu) e insere na hash e no bloom
// ela insere somente se o usuario ainda nao estiver cadastrado, e conta quantos foram inseridos

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
            strcpy(nome_arquivo, "data/usuarios_1000.txt");
            break;

        case 2:
            strcpy(nome_arquivo, "data/usuarios_10000.txt");
            break;

        case 3:
            strcpy(nome_arquivo, "data/usuarios_100000.txt");
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

// cria a tabela hash e o filtro de bloom 
   hash = hash_criar(TAMANHO_HASH);
   bloom = bloom_criar(TAMANHO_BLOOM, NUM_FUNCOES_BLOOM);

   // verifica se as estruturas foram criadas com sucesso
    if (hash == NULL || bloom == NULL) {
    printf("Erro ao criar as estruturas de dados.\n");
    return 1;
}

    int opcao;
    int continuar = 1;

    // loop principal do programa, exibe o menu e executa a opcao escolhida pelo usuario
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
            executar_experimentos();
            break;
 
        case 6:
            continuar = 0;
            break;
            
            default:
            printf("Opcao invalida!\n");
        }
        
        
    }
    printf("\nPrograma finalizado!\n");


// libera a memoria das estruturas antes de sair
    hash_destruir(hash);
    bloom_destruir(bloom);

    return 0;
}