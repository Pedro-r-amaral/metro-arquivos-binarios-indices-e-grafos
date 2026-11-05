# Metrô: Arquivos Binários, Índices e Grafos

Projeto da disciplina de **Estrutura de Dados II**, escrito em C. O sistema gerencia
estações e linhas de metrô armazenadas em um arquivo binário de registros de tamanho
fixo, com índice primário em disco, e constrói um grafo direcionado ponderado sobre
esses dados para executar algoritmos clássicos (caminho mínimo, árvore geradora
mínima e contagem de ciclos).

## Funcionalidades

O programa lê da entrada padrão o número da funcionalidade e seus parâmetros.

### Parte 1: Arquivo de dados

| # | Descrição | Entrada |
|---|-----------|---------|
| 1 | Converte um CSV em arquivo binário | `1 entrada.csv saida.bin` |
| 2 | Leitura sequencial de todos os registros | `2 arquivo.bin` |
| 3 | Busca por critérios (vários filtros combinados com AND) | `3 arquivo.bin` + buscas |
| 4 | Acesso direto por RRN | `4 arquivo.bin RRN` |

### Parte 2: Índice primário

| # | Descrição | Entrada |
|---|-----------|---------|
| 5 | Cria o índice primário (ordenado por `codEstacao`) | `5 arquivo.bin indice.bin` |
| 6 | Busca indexada (busca binária no índice quando o filtro é só `codEstacao`) | `6 arquivo.bin indice.bin` + buscas |
| 7 | Remoção lógica com pilha de registros removidos | `7 arquivo.bin indice.bin` + remoções |
| 8 | Inserção reaproveitando espaços da pilha de removidos | `8 arquivo.bin indice.bin` + inserções |
| 9 | Atualização de registros diretamente em disco | `9 arquivo.bin indice.bin` + atualizações |

### Parte 3: Grafos

O grafo é montado a partir do arquivo binário: cada estação (pelo nome) é um vértice
e cada conexão é uma aresta, guardada em lista de adjacências. Conexões de integração
entre estações têm peso 0.

| # | Descrição | Entrada |
|---|-----------|---------|
| 10 | Constrói e imprime o grafo | `10 arquivo.bin` |
| 11 | Caminho mínimo (Dijkstra) | `11 arquivo.bin indice.bin nomeEstacao "Origem" nomeEstacao "Destino"` |
| 12 | Árvore geradora mínima (Prim) percorrida em DFS | `12 arquivo.bin indice.bin nomeEstacao "Origem"` |
| 13 | Contagem de ciclos simples a partir de uma estação | `13 arquivo.bin indice.bin nomeEstacao "Origem"` |

## Formato dos arquivos

**Arquivo de dados**
- Cabeçalho de 17 bytes: `status`, `topo` (pilha de removidos), `proxRRN`,
  `nroEstacoes`, `nroParesEstacao`.
- Registros de 80 bytes, com campos fixos e dois campos de tamanho variável
  (`nomeEstacao` e `nomeLinha`). O espaço restante é preenchido com `$`.
- Valores nulos inteiros são representados por `-1`.

**Arquivo de índice**
- Cabeçalho de 1 byte (`status`).
- Registros de 8 bytes (`codEstacao`, `rrn`), ordenados por `codEstacao`.

O campo `status` (`'0'` durante escritas, `'1'` quando consistente) é usado para
detectar arquivos que ficaram corrompidos.

## Estrutura do projeto

```
.
├── src/
│   ├── main.c               # despacha a funcionalidade escolhida
│   ├── funcionalidade01.c   # ... até funcionalidade13.c
│   ├── io_binario.c         # leitura/escrita de registros e cabeçalho
│   ├── impressao.c          # formatação de registros na saída
│   ├── grafo.c              # construção do grafo (lista de adjacências)
│   ├── dijkstra.c           # caminho mínimo
│   ├── prim.c               # AGM + DFS
│   ├── ciclos.c             # ciclos simples (DFS com backtracking)
│   └── fornecidas.c         # funções fornecidas pela disciplina
├── include/                 # headers (.h)
├── Makefile
└── README.md
```

## Algoritmos

- **Dijkstra:** a cada passo escolhe o vértice não visitado de menor distância
  acumulada; empates são resolvidos pelo menor nome de estação.
- **Prim:** trata o grafo como não-direcionado. Critérios de desempate: menor
  distância, depois menor nome do vértice já na árvore, depois menor nome do
  vértice de destino. A árvore resultante é percorrida em DFS a partir da origem,
  imprimindo `pai, filho, distancia`.
- **Ciclos simples:** DFS com backtracking a partir da origem, contando os caminhos
  que retornam a ela sem repetir vértices. Imprime `-1` se não há caminho de volta.

## Como compilar e executar

Requisitos: `gcc` e `make` (Linux, macOS ou WSL no Windows).

```bash
make            # gera o executável ./programa
make clean      # remove arquivos compilados
```

O programa lê a entrada padrão:

```bash
./programa < entrada.txt
```

Exemplo de entrada para a funcionalidade 11:

```
11 estacoes.bin indice.bin nomeEstacao "Luz" nomeEstacao "Republica"
```

## Observações

- Projeto acadêmico desenvolvido para a disciplina de Estrutura de Dados II.
- O enunciado e os casos de teste da disciplina não estão incluídos neste repositório.
