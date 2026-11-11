#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"
#include "impressao.h"
#include "fornecidas.h"

void criterioBusca(char *nomeArquivoBin) {
    FILE *fp = fopen(nomeArquivoBin, "rb");
    if (fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //verifica status
    char status;
    fread(&status, sizeof(char), 1, fp);
    if (status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp);
        return;
    }

    //quantidade de buscas
    int nBuscas;
    scanf("%d", &nBuscas);

    for (int i = 0; i < nBuscas; i++) {
        int mFiltros;
        scanf("%d", &mFiltros);

        //guardar filtros de busca
        char nomesCampos[10][50];
        char valoresStrings[10][100];
        int valoresInts[10];
        int buscaPorId = 0;

        //criterios das buscas
        for (int j = 0; j < mFiltros; j++) {
            scanf("%s", nomesCampos[j]);
            if (strcmp(nomesCampos[j], "nomeEstacao") == 0 || strcmp(nomesCampos[j], "nomeLinha") == 0) {
                ScanQuoteString(valoresStrings[j]);
            } else {
                char temp[50];
                scanf("%s", temp);
                if (strcmp(temp, "NULO") == 0) valoresInts[j] = VALOR_NULO_INT;
                else valoresInts[j] = atoi(temp);
                
                if (strcmp(nomesCampos[j], "codEstacao") == 0) {
                    buscaPorId = 1;
                }
            }
        }

        //volta para o inicio dos dados
        fseek(fp, TAM_CABECALHO, SEEK_SET);
        
        Registro reg;
        int encontrouAlgum = 0;

        //leitura do arquivo registro por registro
        while (ler_registro_binario(fp, &reg)) {
            if (reg.removido == '1') {
                if (reg.nomeEstacao) free(reg.nomeEstacao);
                if (reg.nomeLinha) free(reg.nomeLinha);
                continue;
            }

            //verifica se satisfaz todos os filtros de bsuca
            int atendeTodos = 1;
            for (int j = 0; j < mFiltros; j++) {
                if (strcmp(nomesCampos[j], "codEstacao") == 0 && reg.codEstacao != valoresInts[j]) atendeTodos = 0;
                else if (strcmp(nomesCampos[j], "codLinha") == 0 && reg.codLinha != valoresInts[j]) atendeTodos = 0;
                else if (strcmp(nomesCampos[j], "codProxEstacao") == 0 && reg.codProxEstacao != valoresInts[j]) atendeTodos = 0;
                else if (strcmp(nomesCampos[j], "distProxEstacao") == 0 && reg.distProxEstacao != valoresInts[j]) atendeTodos = 0;
                else if (strcmp(nomesCampos[j], "codLinhaIntegra") == 0 && reg.codLinhaIntegra != valoresInts[j]) atendeTodos = 0;
                else if (strcmp(nomesCampos[j], "codEstIntegra") == 0 && reg.codEstIntegra != valoresInts[j]) atendeTodos = 0;
                else if (strcmp(nomesCampos[j], "nomeEstacao") == 0) {
                    if (valoresStrings[j][0] == '\0') { // Usuário buscou por NULO
                        if (reg.nomeEstacao != NULL) atendeTodos = 0;
                    } else { // Usuário buscou uma string real
                        if (reg.nomeEstacao == NULL || strcmp(reg.nomeEstacao, valoresStrings[j]) != 0) 
                            atendeTodos = 0;
                    }
                }
                else if (strcmp(nomesCampos[j], "nomeLinha") == 0) {
                   if (valoresStrings[j][0] == '\0') { // Usuário buscou por NULO
                        if (reg.nomeLinha != NULL) atendeTodos = 0;
                    } else { // Usuário buscou uma string real
                        if (reg.nomeLinha == NULL || strcmp(reg.nomeLinha, valoresStrings[j]) != 0) 
                            atendeTodos = 0;
                    }
                }
                if (!atendeTodos) break;
            }

            if (atendeTodos) {
                imprimir_registro(&reg);
                encontrouAlgum = 1;
            }

            if (reg.nomeEstacao) free(reg.nomeEstacao);
            if (reg.nomeLinha) free(reg.nomeLinha);

            // --- Interrompe a varredura se a busca continha a chave (codEstacao) e ela já foi encontrada ---
            if (atendeTodos && buscaPorId) {
                break;
            }
        }

        if (!encontrouAlgum) {
            printf("Registro inexistente.\n");
        }

        if (i < nBuscas - 1) {  //só imprime \n se nao for a última busca
            printf("\n");
        }
    }

    fclose(fp);
}