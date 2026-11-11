#ifndef PRIM_H
#define PRIM_H

#include "grafo.h"

/* ============================================================
   FUNCIONALIDADE [12] — Árvore Geradora Mínima (Prim) + DFS

   Trata o grafo como não-direcionado, executa o algoritmo de
   Prim para obter a AGM de menor custo total e percorre a AGM
   com DFS a partir do vértice de entrada, imprimindo cada
   aresta visitada no formato: pai, filho, distancia
   ============================================================ */

/* Executa Prim, constrói a AGM e a percorre com DFS.
   nomeOrigem é o nome da estação de partida.
   Retorna 1 se OK, 0 se erro (estação não encontrada). */
int prim(Grafo *g, const char *nomeOrigem);

#endif