#include "grafo.h"
#include <stdio.h>

/* ============================================================
   FUNCIONALIDADE [10]
   Lê o arquivo binário, constrói o grafo direcionado ponderado
   com listas de adjacências e exibe sua estrutura na saída padrão.
   ============================================================ */

void funcionalidade10(void) {
    char arquivoBin[100];
    scanf("%s", arquivoBin);

    Grafo g;
    if (!construirGrafo(arquivoBin, &g)) {
        printf("Falha na execução da funcionalidade.\n");
        return;
    }

    imprimirGrafo(&g);
    liberarGrafo(&g);
}