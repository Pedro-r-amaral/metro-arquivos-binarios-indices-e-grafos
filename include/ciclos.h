#ifndef CICLOS_H
#define CICLOS_H

#include "grafo.h"

/* ============================================================
   FUNCIONALIDADE [13] — Contagem de ciclos simples

   A partir de um vértice de origem no grafo direcionado,
   conta quantos ciclos simples existem, ou seja, caminhos
   que partem da origem e retornam a ela sem repetir vértices.

   Se não houver nenhum caminho de volta à origem a partir de
   qualquer vizinho dela, imprime -1.
   Caso contrário, imprime a quantidade de ciclos encontrados
   (podendo ser 0 se existem caminhos mas nenhum fecha ciclo).
   ============================================================ */

/* Executa a contagem de ciclos simples a partir de nomeOrigem.
   Imprime o resultado no formato: "Quantidade de ciclos: X"
   ou "-1" se não for possível retornar à origem.
   Retorna 1 se OK, 0 se a estação de origem não for encontrada. */
int contarCiclos(Grafo *g, const char *nomeOrigem);

#endif