#include "grafo.h"
#include "dijkstra.h"
#include <stdio.h>

/* ============================================================
   FUNCIONALIDADE [11]
   Lê o arquivo binário, constrói o grafo e executa o algoritmo
   de Dijkstra para encontrar o caminho mínimo entre duas estações.

   Formato de entrada:
   11 arquivo.bin arquivo_indice.bin nomeEstacao "Origem" nomeEstacao "Destino"
   ============================================================ */

void funcionalidade11(void) {
    char arquivoBin[100];
    char arquivoIndice[100]; /* segundo arquivo — lido mas não usado no grafo */
    char campo1[50], valorOrigem[100];
    char campo2[50], valorDestino[100];

    scanf("%s %s %s \"%[^\"]\" %s \"%[^\"]\"",
          arquivoBin, arquivoIndice,
          campo1, valorOrigem,
          campo2, valorDestino);

    Grafo g;
    if (!construirGrafo(arquivoBin, &g)) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    if (!dijkstra(&g, valorOrigem, valorDestino)) {
        printf("Falha na execução da funcionalidade.\n");
    }

    liberarGrafo(&g);
}