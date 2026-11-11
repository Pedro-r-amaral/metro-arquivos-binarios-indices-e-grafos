#include "prim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ============================================================
   ESTRUTURA AUXILIAR — Aresta candidata para o Prim
   Representa uma aresta não-direcionada (u, v, distancia)
   que pode ser adicionada à AGM.
   ============================================================ */
typedef struct {
    int u;        /* índice do vértice de origem (já na AGM) */
    int v;        /* índice do vértice destino (ainda fora da AGM) */
    int distancia;
} ArestaAGM;

/* ============================================================
   ESTRUTURA AUXILIAR — Nó da lista de adjacências da AGM
   Usada para representar a árvore resultante do Prim antes
   de percorrê-la com DFS.
   ============================================================ */
typedef struct NoAGM {
    int vizinho;        /* índice do vértice vizinho na AGM */
    int distancia;      /* peso da aresta */
    struct NoAGM *prox; /* próximo nó da lista */
} NoAGM;

/* ============================================================
   FUNÇÕES AUXILIARES INTERNAS
   ============================================================ */

/* Insere um vizinho na lista de adjacências da AGM do vértice idx,
   mantendo a lista ordenada de forma crescente por nome de estação.
   Como a AGM é não-direcionada, cada aresta é inserida nos dois lados. */
static void inserirVizinhoAGM(NoAGM **listaAGM, Grafo *g, int idx, int vizinho, int dist) {
    NoAGM *novo = malloc(sizeof(NoAGM));
    novo->vizinho  = vizinho;
    novo->distancia = dist;
    novo->prox     = NULL;

    /* inserção em ordem crescente por nome do vértice vizinho */
    NoAGM *cur = listaAGM[idx];
    NoAGM *ant = NULL;
    while (cur != NULL &&
           strcmp(g->vertices[cur->vizinho].nomeEstacao,
                  g->vertices[vizinho].nomeEstacao) < 0) {
        ant = cur;
        cur = cur->prox;
    }

    novo->prox = cur;
    if (ant == NULL) listaAGM[idx] = novo;
    else             ant->prox = novo;
}

/* Escolhe a melhor aresta candidata a ser adicionada à AGM segundo
   os critérios de desempate do enunciado:
   1. menor distância
   2. empate → menor nome do vértice u (origem, já na AGM)
   3. empate → menor nome do vértice v (destino, fora da AGM)
   Percorre todo o vetor de candidatos e retorna o índice do melhor,
   ou -1 se não houver nenhum candidato válido. */
static int escolherMelhorAresta(ArestaAGM *candidatos, int numCandidatos,
                                 int *naAGM, Grafo *g) {
    int melhor = -1;

    for (int i = 0; i < numCandidatos; i++) {
        /* ignora arestas cujo destino já entrou na AGM */
        if (naAGM[candidatos[i].v]) continue;

        if (melhor == -1) {
            melhor = i;
            continue;
        }

        ArestaAGM *m = &candidatos[melhor];
        ArestaAGM *c = &candidatos[i];

        /* Critério 1: menor distância */
        if (c->distancia < m->distancia) {
            melhor = i;
            continue;
        }
        if (c->distancia > m->distancia) continue;

        /* Critério 2: empate em dist → menor nome de u */
        int cmpU = strcmp(g->vertices[c->u].nomeEstacao,
                          g->vertices[m->u].nomeEstacao);
        if (cmpU < 0) {
            melhor = i;
            continue;
        }
        if (cmpU > 0) continue;

        /* Critério 3: empate em u → menor nome de v */
        int cmpV = strcmp(g->vertices[c->v].nomeEstacao,
                          g->vertices[m->v].nomeEstacao);
        if (cmpV < 0) melhor = i;
    }

    return melhor;
}

/* DFS recursiva sobre a AGM.
   Para cada aresta (pai → filho) visitada, imprime:
   "nomePai, nomeFilho, distancia"
   Os filhos já estão ordenados por nome na lista da AGM.

   Além de evitar retornar ao pai, mantém um vetor de visitados: como a
   AGM é armazenada como lista de adjacências não-direcionada (cada
   aresta aparece nos dois sentidos), checar apenas "!= pai" não é
   suficiente para garantir que cada vértice seja visitado uma única vez
   (por exemplo, quando há arestas de peso 0 ou múltiplas ligações entre
   o mesmo par de vértices). O vetor de visitados garante que cada
   vértice da AGM seja impresso e percorrido exatamente uma vez,
   eliminando linhas repetidas na saída. */
static void dfsPrim(int u, int pai, NoAGM **listaAGM, Grafo *g, int *visitado) {
    visitado[u] = 1;

    NoAGM *cur = listaAGM[u];
    while (cur != NULL) {
        /* evita voltar ao pai e evita revisitar vértices já percorridos */
        if (cur->vizinho != pai && !visitado[cur->vizinho]) {
            printf("%s, %s, %d\n",
                   g->vertices[u].nomeEstacao,
                   g->vertices[cur->vizinho].nomeEstacao,
                   cur->distancia);
            dfsPrim(cur->vizinho, u, listaAGM, g, visitado);
        }
        cur = cur->prox;
    }
}

/* ============================================================
   FUNÇÃO PRINCIPAL — Prim + DFS
   ============================================================ */
int prim(Grafo *g, const char *nomeOrigem) {
    int origem = buscarVertice(g, nomeOrigem);
    if (origem < 0) return 0;

    int n = g->numVertices;

    /* naAGM[i] = 1 se o vértice i já foi incorporado à AGM */
    int *naAGM = calloc(n, sizeof(int));

    /* lista de adjacências da AGM resultante (não-direcionada) */
    NoAGM **listaAGM = calloc(n, sizeof(NoAGM *));

    /* vetor de candidatos: cada aresta (u,v) com v ainda fora da AGM.
       No pior caso, n*(n-1) arestas; usamos realoc incremental. */
    int capCandidatos = 64;
    int numCandidatos = 0;
    ArestaAGM *candidatos = malloc(sizeof(ArestaAGM) * capCandidatos);

    /* inicia o Prim adicionando a origem à AGM e suas arestas ao pool */
    naAGM[origem] = 1;

    /* adiciona ao pool todas as arestas que partem de 'origem'
       no grafo original (direcionado) e também as que chegam nele,
       pois o grafo é tratado como não-direcionado aqui */
    for (int i = 0; i < n; i++) {
        if (i == origem) continue;

        /* arestas saindo de origem para i */
        NoAresta *ar = g->vertices[origem].lista;
        while (ar != NULL) {
            int v = buscarVertice(g, ar->nomeProxEstacao);
            if (v == i) {
                if (numCandidatos == capCandidatos) {
                    capCandidatos *= 2;
                    candidatos = realloc(candidatos, sizeof(ArestaAGM) * capCandidatos);
                }
                candidatos[numCandidatos].u         = origem;
                candidatos[numCandidatos].v         = i;
                candidatos[numCandidatos].distancia = ar->distancia;
                numCandidatos++;
                break; /* uma aresta por par (u,v) no pool — a menor já foi inserida */
            }
            ar = ar->prox;
        }

        /* arestas saindo de i para origem (sentido inverso) */
        ar = g->vertices[i].lista;
        while (ar != NULL) {
            int v = buscarVertice(g, ar->nomeProxEstacao);
            if (v == origem) {
                if (numCandidatos == capCandidatos) {
                    capCandidatos *= 2;
                    candidatos = realloc(candidatos, sizeof(ArestaAGM) * capCandidatos);
                }
                /* no grafo não-direcionado, a aresta (i→origem) vira (origem, i) */
                candidatos[numCandidatos].u         = origem;
                candidatos[numCandidatos].v         = i;
                candidatos[numCandidatos].distancia = ar->distancia;
                numCandidatos++;
                break;
            }
            ar = ar->prox;
        }
    }

    /* loop principal do Prim: adiciona n-1 arestas à AGM */
    for (int iter = 0; iter < n - 1; iter++) {
        int melhor = escolherMelhorAresta(candidatos, numCandidatos, naAGM, g);
        if (melhor == -1) break; /* grafo desconexo — para aqui */

        int u    = candidatos[melhor].u;
        int v    = candidatos[melhor].v;
        int dist = candidatos[melhor].distancia;

        /* incorpora v à AGM */
        naAGM[v] = 1;

        /* registra a aresta nos dois sentidos na lista da AGM */
        inserirVizinhoAGM(listaAGM, g, u, v, dist);
        inserirVizinhoAGM(listaAGM, g, v, u, dist);

        /* adiciona ao pool todas as arestas incidentes em v
           cujo destino ainda está fora da AGM */
        for (int i = 0; i < n; i++) {
            if (naAGM[i]) continue;

            /* aresta v → i (direção original) */
            NoAresta *ar = g->vertices[v].lista;
            while (ar != NULL) {
                int w = buscarVertice(g, ar->nomeProxEstacao);
                if (w == i) {
                    if (numCandidatos == capCandidatos) {
                        capCandidatos *= 2;
                        candidatos = realloc(candidatos, sizeof(ArestaAGM) * capCandidatos);
                    }
                    candidatos[numCandidatos].u         = v;
                    candidatos[numCandidatos].v         = i;
                    candidatos[numCandidatos].distancia = ar->distancia;
                    numCandidatos++;
                    break;
                }
                ar = ar->prox;
            }

            /* aresta i → v (sentido inverso, tratado como v → i no não-direcionado) */
            ar = g->vertices[i].lista;
            while (ar != NULL) {
                int w = buscarVertice(g, ar->nomeProxEstacao);
                if (w == v) {
                    if (numCandidatos == capCandidatos) {
                        capCandidatos *= 2;
                        candidatos = realloc(candidatos, sizeof(ArestaAGM) * capCandidatos);
                    }
                    candidatos[numCandidatos].u         = v;
                    candidatos[numCandidatos].v         = i;
                    candidatos[numCandidatos].distancia = ar->distancia;
                    numCandidatos++;
                    break;
                }
                ar = ar->prox;
            }
        }
    }

    /* percorre a AGM com DFS a partir da origem e imprime as arestas */
    int *visitado = calloc(n, sizeof(int));
    dfsPrim(origem, -1, listaAGM, g, visitado);
    free(visitado);

    /* libera memória auxiliar */
    free(candidatos);
    free(naAGM);
    for (int i = 0; i < n; i++) {
        NoAGM *cur = listaAGM[i];
        while (cur) {
            NoAGM *prox = cur->prox;
            free(cur);
            cur = prox;
        }
    }
    free(listaAGM);

    return 1;
}