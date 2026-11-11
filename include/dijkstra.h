#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "grafo.h"

/* ============================================================
   FUNCIONALIDADE [11] — Caminho mínimo (Dijkstra)
   
   Dado um grafo g, uma estação de origem e uma de destino,
   calcula e imprime o caminho de menor distância entre elas.
   ============================================================ */

/* Executa Dijkstra e imprime o resultado no formato exigido.
   nomeOrigem e nomeDestino são os nomes das estações.
   Retorna 1 se OK, 0 se erro (estação não encontrada, etc.). */
int dijkstra(Grafo *g, const char *nomeOrigem, const char *nomeDestino);

#endif