#ifndef GRAFO_H
#define GRAFO_H

#include "registros.h"

/* ============================================================
   STRUCTS DO GRAFO
   Representação: lista de adjacências
   - Vetor de vértices ordenado crescente por nomeEstacao
   - Cada vértice aponta para uma lista encadeada de arestas
   ============================================================ */

/* Nó da lista encadeada de arestas.
   Representa uma aresta (u → v) onde u é o vértice dono da lista. */
typedef struct NoAresta {
    char *nomeProxEstacao;  /* nome do vértice destino */
    int distancia;          /* peso da aresta (0 para integração) */
    char **linhas;          /* vetor de nomes de linhas (pode ter várias) */
    int numLinhas;          /* quantidade de linhas neste elemento */
    struct NoAresta *prox;  /* próximo elemento da lista */
} NoAresta;

/* Vértice do grafo: representa uma estação única */
typedef struct {
    char *nomeEstacao;   /* nome da estação (chave de ordenação) */
    NoAresta *lista;     /* cabeça da lista de arestas adjacentes */
} Vertice;

/* Grafo completo */
typedef struct {
    Vertice *vertices;   /* vetor de vértices ordenado por nome */
    int numVertices;     /* tamanho do vetor */
} Grafo;

/* ============================================================
   PROTÓTIPOS
   ============================================================ */

/* Constrói o grafo lendo o arquivo binário. Retorna 1 se OK, 0 se erro. */
int construirGrafo(const char *nomeArquivo, Grafo *g);

/* Imprime o grafo no formato exigido pela funcionalidade [10] */
void imprimirGrafo(Grafo *g);

/* Libera toda a memória alocada pelo grafo */
void liberarGrafo(Grafo *g);

/* Busca um vértice por nome no vetor (busca binária). Retorna índice ou -1. */
int buscarVertice(Grafo *g, const char *nome);

#endif