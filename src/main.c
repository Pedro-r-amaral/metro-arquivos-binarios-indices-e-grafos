#include <stdio.h>
#include <stdlib.h>
#include "registros.h"
#include "fornecidas.h"


int main() {
    int funcao;
    char arquivoEntrada[100];
    char arquivoSaida[100];

    /* ler qual funcao o usuário quer executar */
    if (scanf("%d", &funcao) != 1) {
        return 0;
    }

    switch (funcao) {
        case 1:
            /* formato: 1 arquivoEntrada.csv arquivoSaida.bin */
            scanf("%s %s", arquivoEntrada, arquivoSaida);
            if (CSVparaBin(arquivoEntrada, arquivoSaida)) {
                BinarioNaTela(arquivoSaida);
            }
            break;

        case 2:
            /* formato: 2 arquivoEntrada.bin */
            scanf("%s", arquivoEntrada);
            leitorSequencial(arquivoEntrada);
            break;

        case 3:
            /* formato: 3 arquivoEntrada.bin */
            scanf("%s", arquivoEntrada);
            criterioBusca(arquivoEntrada);
            break;

        case 4: {
            /* formato: 4 arquivoEntrada.bin RRN */
            int rrn;
            scanf("%s %d", arquivoEntrada, &rrn);
            acessoDiretoRRN(arquivoEntrada, rrn);
            break;
        }

        case 5: {
            /* formato: 5 arquivoEntrada.bin arquivoIndice.bin */
            char arquivoIndice[100];
            scanf("%s %s", arquivoEntrada, arquivoIndice);
            if (criarIndicePrimario(arquivoEntrada, arquivoIndice)) {
                BinarioNaTela(arquivoIndice);
            }
            break;
        }

        case 6: {
            /* formato: 6 arquivoEntrada.bin arquivoIndice.bin ... */
            char arquivoIndice[100];
            scanf("%s %s", arquivoEntrada, arquivoIndice);
            buscaIndexada(arquivoEntrada, arquivoIndice);
            break;
        }

        case 7: {
            /* formato: 7 arquivoEntrada.bin arquivoIndice.bin ... */
            char arquivoIndice[100];
            scanf("%s %s", arquivoEntrada, arquivoIndice);
            remocaoLogica(arquivoEntrada, arquivoIndice);
            break;
        }

        case 8: {
            /* formato: 8 arquivoEntrada.bin arquivoIndice.bin ... */
            char arquivoIndice[100];
            scanf("%s %s", arquivoEntrada, arquivoIndice);
            insercaoDados(arquivoEntrada, arquivoIndice);
            break;
        }

        case 9: {
            /* formato: 9 arquivoEntrada.bin arquivoIndice.bin ... */
            char arquivoIndice[100];
            scanf("%s %s", arquivoEntrada, arquivoIndice);
            atualizacaoDados(arquivoEntrada, arquivoIndice);
            break;
        }

        case 10:{
            /* formato: 10 arquivoEntrada.bin */
            funcionalidade10();
            break;
        }
        case 11:{
            /* formato: 11 arquivo.bin nomeEstacaoOrigem "Valor" nomeEstacaoDestino "Valor" */
            funcionalidade11();
            break;
        }
        case 12:{
            /* formato: 12 arquivoEntrada.bin arquivoIndice.bin nomeEstacao "valorOrigem" */
            funcionalidade12();
            break;
        }
        case 13:{
            /* formato: 13 arquivoEntrada.bin arquivoIndice.bin nomeEstacao "valorOrigem" */
            funcionalidade13();
            break;
        }
        default:
            printf("funcionalidade invalida.\n");
            break;
    }

    return 0;
}