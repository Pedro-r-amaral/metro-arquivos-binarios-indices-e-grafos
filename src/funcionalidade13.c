#include "grafo.h"
#include "ciclos.h"
#include <stdio.h>

/* ============================================================
   FUNCIONALIDADE [13]
   Lê o arquivo binário, constrói o grafo direcionado e conta
   quantos ciclos simples existem a partir do vértice de origem.

   Imprime "Quantidade de ciclos: X" onde X é o número de
   ciclos encontrados, ou -1 se não houver caminho de volta
   à origem a partir de nenhum de seus vizinhos.

   Formato de entrada:
   13 arquivoEntrada.bin nomeEstacaoOrigem "valorOrigem"
   ============================================================ */

void funcionalidade13(void) {
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

    if (!contarCiclos(&g, valorOrigem)) {
        printf("Falha na execução da funcionalidade.\n");
    }

    liberarGrafo(&g);
}