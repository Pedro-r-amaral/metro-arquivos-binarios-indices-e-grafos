#include "dijkstra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* ============================================================
   DIJKSTRA — caminho mínimo em grafo direcionado ponderado
   ============================================================ */

/* Escolhe o próximo vértice a visitar:
   - não visitado
   - menor distância acumulada
   - empate: menor nome de estação
   Retorna o índice escolhido, ou -1 se todos visitados. */
static int escolherMinimo(Grafo *g, int *dist, int *visitado) {
    int melhor = -1;

    for (int i = 0; i < g->numVertices; i++) {
        if (visitado[i]) continue;
        if (dist[i] == INT_MAX) continue;

        if (melhor == -1) {
            melhor = i;
            continue;
        }

        /* Critério 1: menor distância */
        if (dist[i] < dist[melhor]) {
            melhor = i;
            continue;
        }

        /* Critério 2: empate em distância → menor nome */
        if (dist[i] == dist[melhor] &&
            strcmp(g->vertices[i].nomeEstacao,
                   g->vertices[melhor].nomeEstacao) < 0) {
            melhor = i;
        }
    }
    return melhor;
}

/* ============================================================
   FUNÇÃO PRINCIPAL
   ============================================================ */
int dijkstra(Grafo *g, const char *nomeOrigem, const char *nomeDestino) {

    /* Localizar origem e destino no vetor de vértices */
    int origem  = buscarVertice(g, nomeOrigem);
    int destino = buscarVertice(g, nomeDestino);

    if (origem < 0 || destino < 0) return 0;

    int n = g->numVertices;

    /* Alocar vetores auxiliares */
    int *dist      = malloc(sizeof(int) * n); /* distância mínima acumulada */
    int *visitado  = malloc(sizeof(int) * n); /* 1 se já finalizado         */
    int *pred      = malloc(sizeof(int) * n); /* índice do predecessor       */

    for (int i = 0; i < n; i++) {
        dist[i]     = INT_MAX;
        visitado[i] = 0;
        pred[i]     = -1;
    }
    dist[origem] = 0;

    /* Loop principal: n iterações */
    for (int iter = 0; iter < n; iter++) {

        /* Escolher vértice u com menor dist não visitado */
        int u = escolherMinimo(g, dist, visitado);
        if (u == -1) break; /* todos os alcançáveis já foram processados */

        visitado[u] = 1;

        /* Relaxar arestas saindo de u */
        NoAresta *aresta = g->vertices[u].lista;
        while (aresta != NULL) {
            /* Encontrar índice do vértice destino v */
            int v = buscarVertice(g, aresta->nomeProxEstacao);
            if (v >= 0 && !visitado[v]) {
                int novaDist = dist[u] + aresta->distancia;

                if (novaDist < dist[v]) {
                    dist[v] = novaDist;
                    pred[v] = u;
                }
                /* Empate em distância: escolhe aresta com menor u
                   (u já é fixo aqui; se mesma dist veio de u diferente,
                    o menor u já foi escolhido por escolherMinimo antes) */
                else if (novaDist == dist[v] && pred[v] != -1) {
                    /* menor u: índice u atual vs pred[v] atual */
                    int uAtual = u;
                    int uAnterior = pred[v];
                    int cmpU = strcmp(g->vertices[uAtual].nomeEstacao,
                                      g->vertices[uAnterior].nomeEstacao);
                    if (cmpU < 0) {
                        pred[v] = u;
                    } else if (cmpU == 0) {
                        /* empate em u: escolhe menor v (nome do destino) */
                        /* v é o mesmo nos dois casos, então não há o que mudar */
                    }
                }
            }
            aresta = aresta->prox;
        }
    }

    /* Verificar se destino é alcançável */
    if (dist[destino] == INT_MAX) {
        printf("Nao existe caminho entre as estacoes solicitadas.\n");
        free(dist); free(visitado); free(pred);
        return 1; /* não é erro de execução, só não há caminho */
    }

    /* Reconstruir caminho de trás para frente */
    int *caminho = malloc(sizeof(int) * n);
    int  tamCaminho = 0;
    int  cur = destino;

    while (cur != -1) {
        caminho[tamCaminho++] = cur;
        cur = pred[cur];
    }

    /* Inverter para ordem origem → destino */
    for (int i = 0; i < tamCaminho / 2; i++) {
        int tmp = caminho[i];
        caminho[i] = caminho[tamCaminho - 1 - i];
        caminho[tamCaminho - 1 - i] = tmp;
    }

    /* Imprimir resultado */
    /* número de estações percorridas (exclui origem, inclui destino) */
    printf("Numero de estacoes que serao percorridas: %d\n", tamCaminho - 1);
    printf("Distancia que sera percorrida: %d\n", dist[destino]);

    /* caminho: estações separadas por ", " */
    for (int i = 0; i < tamCaminho; i++) {
        if (i > 0) printf(", ");
        printf("%s", g->vertices[caminho[i]].nomeEstacao);
    }
    printf("\n");

    free(dist);
    free(visitado);
    free(pred);
    free(caminho);
    return 1;
}