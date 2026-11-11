#include "ciclos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   CONTAGEM DE CICLOS SIMPLES — DFS com backtracking
   ============================================================ */

/* Contexto compartilhado entre as chamadas recursivas da DFS.
   Evita passar muitos parâmetros e facilita o acesso ao estado global. */
typedef struct {
    Grafo *g;         /* grafo sobre o qual a DFS opera              */
    int    origem;    /* índice do vértice de origem (fixo)          */
    int   *visitado;  /* visitado[i] = 1 se i está no caminho atual  */
    int    contador;  /* quantidade de ciclos simples encontrados     */
    int    podVoltar; /* 1 se existe ao menos um caminho à origem     */
} ContextoDFS;

/* Verifica se existe algum caminho direto da origem para ela mesma,
   ou seja, se ao menos um vizinho da origem pode alcançar a origem.
   Essa verificação é feita com uma DFS simples de alcançabilidade:
   para cada vizinho v da origem, testa se é possível chegar à origem
   partindo de v sem sair do grafo.
   Retorna 1 se algum caminho de volta existe, 0 caso contrário. */
static int verificarAlcancabilidade(Grafo *g, int v, int origem, int *marcado) {
    marcado[v] = 1;

    NoAresta *ar = g->vertices[v].lista;
    while (ar != NULL) {
        int w = buscarVertice(g, ar->nomeProxEstacao);
        if (w == origem) return 1; /* achou caminho de volta à origem */
        if (w >= 0 && !marcado[w]) {
            if (verificarAlcancabilidade(g, w, origem, marcado)) return 1;
        }
        ar = ar->prox;
    }
    return 0;
}

/* DFS recursiva com backtracking para contar ciclos simples.
   Percorre todos os caminhos simples que partem do vértice atual (u)
   e chegam à origem sem repetir vértices intermediários.
   Cada vez que a origem é alcançada, incrementa ctx->contador. */
static void dfsCiclos(ContextoDFS *ctx, int u) {
    NoAresta *ar = ctx->g->vertices[u].lista;

    while (ar != NULL) {
        int v = buscarVertice(ctx->g, ar->nomeProxEstacao);

        if (v >= 0) {
            if (v == ctx->origem) {
                /* fechou um ciclo simples: caminho atual + aresta de volta */
                ctx->contador++;
            } else if (!ctx->visitado[v]) {
                /* ainda não visitado: expande o caminho e continua a DFS */
                ctx->visitado[v] = 1;
                dfsCiclos(ctx, v);
                ctx->visitado[v] = 0; /* backtracking */
            }
        }

        ar = ar->prox;
    }
}

/* ============================================================
   FUNÇÃO PRINCIPAL
   ============================================================ */
int contarCiclos(Grafo *g, const char *nomeOrigem) {
    int origem = buscarVertice(g, nomeOrigem);
    if (origem < 0) return 0;

    int n = g->numVertices;

    /* vetor de visitados compartilhado pela DFS (todos livres no início) */
    int *visitado = calloc(n, sizeof(int));

    /* --- Verificar se é possível retornar à origem ---
       Para cada vizinho direto da origem, testa alcançabilidade de volta.
       Se nenhum vizinho consegue alcançar a origem, a resposta é -1. */
    int podVoltar = 0;
    NoAresta *ar = g->vertices[origem].lista;
    while (ar != NULL) {
        int v = buscarVertice(g, ar->nomeProxEstacao);
        if (v == origem) {
            /* self-loop: a própria origem aponta para si mesma */
            podVoltar = 1;
            break;
        }
        if (v >= 0) {
            /* marcado[] isolado por chamada para não interferir entre vizinhos */
            int *marcado = calloc(n, sizeof(int));
            if (verificarAlcancabilidade(g, v, origem, marcado)) {
                podVoltar = 1;
            }
            free(marcado);
            if (podVoltar) break;
        }
        ar = ar->prox;
    }

    if (!podVoltar) {
        printf("Quantidade de ciclos: -1\n");
        free(visitado);
        return 1;
    }

    /* --- Contar ciclos simples com DFS + backtracking ---
       Marca a origem como visitada para que os caminhos
       não passem por ela no meio (só ao fechar o ciclo). */
    ContextoDFS ctx;
    ctx.g        = g;
    ctx.origem   = origem;
    ctx.visitado = visitado;
    ctx.contador = 0;
    ctx.podVoltar = podVoltar;

    visitado[origem] = 1;
    dfsCiclos(&ctx, origem);
    visitado[origem] = 0;

    printf("Quantidade de ciclos: %d\n", ctx.contador);

    free(visitado);
    return 1;
}