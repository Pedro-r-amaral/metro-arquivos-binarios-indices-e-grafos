#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"

//func1
//lida com a fragmentação e campos nulos
char* extrair_campo(char **ptr) {
    if (*ptr == NULL) return "";
    char *campo = strsep(ptr, ",");
    if (campo == NULL) return "";
    
    //remove \n ou \r
    campo[strcspn(campo, "\r\n")] = '\0';
    return campo;
}

int CSVparaBin(char *nomeArquivoCSV, char *nomeArquivoBin) {
    FILE *fCSV = fopen(nomeArquivoCSV, "r");
    if (!fCSV) {
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }

    FILE *fBin = fopen(nomeArquivoBin, "wb");
    if (!fBin) {
        fclose(fCSV);
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }

    //status 0 durante a escrita
    Cabecalho cab = {'0', -1, 0, 0, 0};
    escreverCabecalho(fBin, &cab);

    char linha[1024];
    if (!fgets(linha, 1024, fCSV)) {
        fclose(fCSV);
        fclose(fBin);
        return 0; //pula o cabeçalho do CSV
    }

    // --- estruturas auxiliares dinamicas pra controle de unicidade ---
    int *estacoes_vistas = NULL;
    int qtd_estacoes_vistas = 0;

    typedef struct {
        int cod1;
        int cod2;
    } ParEstacao;
    ParEstacao *pares_vistos = NULL;
    int qtd_pares_vistos = 0;

    while (fgets(linha, 1024, fCSV)) {
        char *ptr = linha;
        Registro reg;
        char *token;

        reg.removido = '0';
        reg.proximo = -1;

        //codEstacao
        token = extrair_campo(&ptr);
        reg.codEstacao = (token[0] == '\0') ? VALOR_NULO_INT : atoi(token);

        //nomeEstacao
        token = extrair_campo(&ptr);
        reg.tamNomeEstacao = strlen(token);
        if (reg.tamNomeEstacao > 0) {
            reg.nomeEstacao = malloc(reg.tamNomeEstacao);
            memcpy(reg.nomeEstacao, token, reg.tamNomeEstacao);
        } else {
            reg.nomeEstacao = NULL;
        }

        //codLinha
        token = extrair_campo(&ptr);
        reg.codLinha = (token[0] == '\0') ? VALOR_NULO_INT : atoi(token);

        // nomeLinha
        token = extrair_campo(&ptr);
        reg.tamNomeLinha = strlen(token);
        if (reg.tamNomeLinha > 0) {
            reg.nomeLinha = malloc(reg.tamNomeLinha);
            memcpy(reg.nomeLinha, token, reg.tamNomeLinha);
        } else {
            reg.nomeLinha = NULL;
        }

        //codProxEstacao
        token = extrair_campo(&ptr);
        reg.codProxEstacao = (token[0] == '\0') ? VALOR_NULO_INT : atoi(token);

        //distProxEstacao
        token = extrair_campo(&ptr);
        reg.distProxEstacao = (token[0] == '\0') ? VALOR_NULO_INT : atoi(token);

        //codLinhaIntegra
        token = extrair_campo(&ptr);
        reg.codLinhaIntegra = (token[0] == '\0') ? VALOR_NULO_INT : atoi(token);

        //codEstIntegra
        token = extrair_campo(&ptr);
        reg.codEstIntegra = (token[0] == '\0') ? VALOR_NULO_INT : atoi(token);

        // --- verificação de estacoes unicas ---
        if (reg.codEstacao != VALOR_NULO_INT) {
            int ja_existe_estacao = 0;
            for (int k = 0; k < qtd_estacoes_vistas; k++) {
                if (estacoes_vistas[k] == reg.codEstacao) {
                    ja_existe_estacao = 1;
                    break;
                }
            }
            if (!ja_existe_estacao) {
                estacoes_vistas = realloc(estacoes_vistas, sizeof(int) * (qtd_estacoes_vistas + 1));
                estacoes_vistas[qtd_estacoes_vistas] = reg.codEstacao;
                qtd_estacoes_vistas++;
                cab.nroEstacoes++;
            }
        }

        // --- verificacao de pares unicos ---
        if (reg.codEstacao != VALOR_NULO_INT && reg.codProxEstacao != VALOR_NULO_INT) {
            int ja_existe_par = 0;
            // Par não direcionado (A-B é igual a B-A)
            for (int k = 0; k < qtd_pares_vistos; k++) {
                if ((pares_vistos[k].cod1 == reg.codEstacao && pares_vistos[k].cod2 == reg.codProxEstacao) ||
                    (pares_vistos[k].cod1 == reg.codProxEstacao && pares_vistos[k].cod2 == reg.codEstacao)) {
                    ja_existe_par = 1;
                    break;
                }
            }
            if (!ja_existe_par) {
                pares_vistos = realloc(pares_vistos, sizeof(ParEstacao) * (qtd_pares_vistos + 1));
                pares_vistos[qtd_pares_vistos].cod1 = reg.codEstacao;
                pares_vistos[qtd_pares_vistos].cod2 = reg.codProxEstacao;
                qtd_pares_vistos++;
                cab.nroParesEstacao++;
            }
        }

        escreverRegistro(fBin, &reg);
        if (reg.nomeEstacao) free(reg.nomeEstacao);
        if (reg.nomeLinha) free(reg.nomeLinha);
        cab.proxRRN++;
    }

    //liberar estruturas auxiliares
    free(estacoes_vistas);
    free(pares_vistos);

    //atualizar status para 1 e salvar cabeçalho final
    cab.status = '1';
    escreverCabecalho(fBin, &cab);

    fclose(fCSV);
    fclose(fBin);
    return 1;
}