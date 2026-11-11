#include "grafo.h"
#include "prim.h"
#include <stdio.h>

/* ============================================================
   FUNCIONALIDADE [12]
   Lê o arquivo binário, constrói o grafo e executa o algoritmo
   de Prim para obter a árvore geradora mínima (AGM). A AGM é
   então percorrida com DFS a partir do vértice de entrada,
   imprimindo cada aresta no formato: pai, filho, distancia.

   Formato de entrada:
   12 arquivoEntrada.bin nomeEstacaoOrigem "valorOrigem"
   ============================================================ */

void funcionalidade12(void) {
    char arquivoBin[100];
    char arquivoIndice[100];
    char campo[50];
    char valorOrigem[100];

    scanf("%s %s %s \"%[^\"]\"", arquivoBin, arquivoIndice, campo, valorOrigem);

    Grafo g;
    if (!construirGrafo(arquivoBin, &g)) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    if (!prim(&g, valorOrigem)) {
        printf("Falha na execução da funcionalidade.\n");
    }

    liberarGrafo(&g);
}