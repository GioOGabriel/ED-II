# 🔍 Sistema de Consulta Eficiente com Tabela Hash e Filtro de Bloom

---

## 📋 Visão Geral

Este projeto implementa um **sistema eficiente de verificação de cadastro de usuários**, capaz de armazenar e consultar grandes volumes de registros com performance otimizada.

São utilizadas duas estruturas de dados complementares:

- **Tabela Hash**: armazenamento e recuperação exata dos elementos em tempo O(1) médio
- **Filtro de Bloom**: estrutura probabilística que filtra consultas desnecessárias à Tabela Hash, reduzindo o tempo total de busca

---

## 🗂️ Estrutura do Projeto

```
ED-II/
│
├── src/
│   ├── main.c               # integração, menu e experimentos
│   ├── hash.c               # implementação da Tabela Hash
│   ├── hash.h               # interface da Tabela Hash
│   ├── bloom.c              # implementação do Filtro de Bloom
│   ├── bloom.h              # interface do Filtro de Bloom
│   └── gerador.c            # gerador dos arquivos de usuários
│
├── data/
│   ├── usuarios_1000.txt    # 1.000 usuários para experimento
│   ├── usuarios_10000.txt   # 10.000 usuários para experimento
│   └── usuarios_100000.txt  # 100.000 usuários para experimento
│
├── testes/
│
├── resultados_experimentos.csv  # gerado automaticamente ao rodar opção 5
├── relatorio.pdf
└── README.md
```

---

## ⚙️ Como Compilar

O executável deve ser gerado a partir da pasta raiz do projeto (`ED-II/`).

### Linux / macOS

```bash
gcc src/main.c src/hash.c src/bloom.c -o programa -lm
```

### Windows (MinGW)

```bash
gcc src/main.c src/hash.c src/bloom.c -o programa.exe -lm
```

> A flag `-lm` é obrigatória pois o Filtro de Bloom usa funções matemáticas (`log`, `pow`, `exp`, `ceil`) da biblioteca `math.h`.

---

## ▶️ Como Executar

```bash
./programa
```

O executável deve ser rodado a partir da pasta raiz do projeto (`ED-II/`), pois os arquivos de dados são lidos com o caminho relativo `data/`.

---

## 📥 Formato de Entrada

O identificador de usuário deve ter **exatamente 11 caracteres**:

- 8 letras minúsculas (a–z)
- 3 dígitos numéricos (0–9)

**Exemplos válidos:**
```
joaosilv123
mariasou456
pedroalv789
```

Os arquivos de usuários seguem o mesmo formato, um identificador por linha:
```
islaifda122
djskalsa297
fjkldsaf881
```

---

## 🖥️ Exemplo de Execução

```
------------------------------
|      ESCOLHA UMA OPCAO      |
------------------------------
[1] Cadastrar novo usuario
[2] Consultar usuarios
[3] Estatisticas
[4] Inserir usuarios em lote
[5] Executar experimentos (1000/10000/100000)
[6] Sair
Opcao: 1

---------------------------
| INSERIR NOVO USUARIO     |
---------------------------
Usuario (8 letras + 3 numeros): joaosilv123

Usuario cadastrado com sucesso!
```

```
Opcao: 2

---------------------
|  CONSULTAR USUARIO |
---------------------
Usuario (8 letras + 3 numeros): joaosilv123

Usuario encontrado!
Usuario: joaosilv123
```

```
Opcao: 2

---------------------
|  CONSULTAR USUARIO |
---------------------
Usuario (8 letras + 3 numeros): xyzabcde999

Usuario nao encontrado!
```

```
Opcao: 3

--- ESTATISTICAS DA TABELA HASH ---
Total de elementos: 1
Total de colisoes: 0
Fator de carga: 0.00

--- ESTATISTICAS DO BLOOM FILTER ---
Tamanho do vetor (bits): 65536
Numero de funcoes hash: 7
Taxa de ocupacao: 0.01%
Falsos positivos encontrados: 0
Probabilidade teorica de falso positivo: 0.000000

--- ESTATISTICAS DAS CONSULTAS MANUAIS ---
Consultas realizadas: 2
Consultas evitadas pelo Bloom: 1
Percentual evitado pelo Bloom: 50.00%
Taxa de falsos positivos: 0.00%
Tempo medio de consulta: 0.000001000 segundos

--- RESULTADO DO ULTIMO EXPERIMENTO AUTOMATICO ---
Experimento automatico ainda nao executado (use a opcao 5).
```

```
Opcao: 5

==================================================================
   EXPERIMENTO: TEMPO SEM BLOOM x TEMPO COM BLOOM x FALSOS POSITIVOS
==================================================================
Quantidade   Tempo sem Bloom(s)   Tempo com Bloom(s)   Falsos Pos(%)   Bits usados  Num funcs
1000         0.000500             0.000800             0.90            9585         7
10000        0.005000             0.007000             1.10            95851        7
100000       0.152000             0.190000             1.05            958506       7

Resultados salvos em 'resultados_experimentos.csv'
```

---

## 🔧 Dimensionamento das Estruturas

### Tabela Hash

- **Tamanho:** 12.007 posições (número primo)
- **Tratamento de colisão:** encadeamento externo (listas ligadas)
- **Função hash:** FNV-1a
- **Fator de carga esperado:** ~0,08 para 1.000 elementos; ~0,83 para 10.000

Números primos como tamanho de tabela reduzem colisões ao distribuir melhor os índices calculados pelo operador `%`.

### Filtro de Bloom (menu interativo)

- **Tamanho:** 65.536 bits
- **Funções hash:** 7 (djb2, sdbm, fnv-1a com variações por seed)
- **Adequado para:** até ~5.000 elementos com taxa de falso positivo abaixo de 1%

### Filtro de Bloom (experimentos automáticos)

No experimento da opção 5, cada cenário usa dimensionamento dinâmico calculado pela fórmula ideal:

```
m = -(n * ln(p)) / (ln(2))²
k = (m / n) * ln(2)
```

Onde `n` é o número de elementos e `p = 0,01` (1% de falso positivo desejado).

---
