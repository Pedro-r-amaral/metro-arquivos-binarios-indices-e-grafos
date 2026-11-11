#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"
#include "impressao.h"
#include "fornecidas.h"

/*
 * Faz busca binaria no arquivo de indice primario procurando pelo codEstacao.
 * O arquivo de indice esta ordenado de forma crescente por codEstacao, entao
 * da pra usar busca binaria direto nos registros do indice (cada um com 8 bytes,
 * vindo logo depois do cabecalho de 1 byte).
 * Retorna o RRN associado aquele codEstacao no arquivo de dados, ou -1 se nao
 * achar nada com esse codigo no indice.
 */
static int busca_binaria_indice(FILE *fIndice, int codBuscado, int nRegistros) {
    int esq = 0, dir = nRegistros - 1;

    while (esq <= dir) {
        int meio = (esq + dir) / 2;

        // posiciona no registro do meio (pulando o cabecalho do indice)
        fseek(fIndice, TAM_CABECALHO_INDICE + meio * TAM_REGISTRO_INDICE, SEEK_SET);

        RegistroIndice ri;
        fread(&ri.codEstacao, sizeof(int32_t), 1, fIndice);
        fread(&ri.rrn,        sizeof(int32_t), 1, fIndice);

        if (ri.codEstacao == codBuscado) return ri.rrn;
        if (ri.codEstacao < codBuscado)  esq = meio + 1;
        else                             dir = meio - 1;
    }

    return -1; // nao encontrado no indice
}

/*
 * Recebe um RRN e le o registro correspondente diretamente do arquivo de dados,
 * sem precisar varrer o arquivo do inicio. O calculo do offset usa o tamanho
 * fixo de cada registro (TAM_REGISTRO_DADOS) somado ao tamanho do cabecalho.
 */
static int ler_registro_por_rrn(FILE *fDados, int rrn, Registro *reg) {
    long offset = (long)TAM_CABECALHO + (long)rrn * TAM_REGISTRO_DADOS;
    fseek(fDados, offset, SEEK_SET);
    return ler_registro_binario(fDados, reg);
}

/*
 * Confere se um registro lido do arquivo atende todos os criterios passados
 * pelo usuario (todos precisam ser verdadeiros, ou seja, e um "AND" entre os
 * filtros). Cada filtro e um par nomeCampo/valorCampo.
 *
 * Para campos string (nomeEstacao e nomeLinha), se o usuario buscou por NULO
 * o valorString vem vazio ("") - nesse caso o registro so satisfaz se o campo
 * dele tambem for NULL.
 */
static int satisfaz_filtros(Registro *reg, int mFiltros,
                            char nomesCampos[][50],
                            char valoresStrings[][100],
                            int  valoresInts[]) {
    for (int j = 0; j < mFiltros; j++) {
        if (strcmp(nomesCampos[j], "codEstacao") == 0) {
            if (reg->codEstacao != valoresInts[j]) return 0;
        } else if (strcmp(nomesCampos[j], "codLinha") == 0) {
            if (reg->codLinha != valoresInts[j]) return 0;
        } else if (strcmp(nomesCampos[j], "codProxEstacao") == 0) {
            if (reg->codProxEstacao != valoresInts[j]) return 0;
        } else if (strcmp(nomesCampos[j], "distProxEstacao") == 0) {
            if (reg->distProxEstacao != valoresInts[j]) return 0;
        } else if (strcmp(nomesCampos[j], "codLinhaIntegra") == 0) {
            if (reg->codLinhaIntegra != valoresInts[j]) return 0;
        } else if (strcmp(nomesCampos[j], "codEstIntegra") == 0) {
            if (reg->codEstIntegra != valoresInts[j]) return 0;
        } else if (strcmp(nomesCampos[j], "nomeEstacao") == 0) {
            // valor vazio = usuario pediu busca por NULO
            if (valoresStrings[j][0] == '\0') {
                if (reg->nomeEstacao != NULL) return 0;
            } else {
                if (reg->nomeEstacao == NULL ||
                    strcmp(reg->nomeEstacao, valoresStrings[j]) != 0) return 0;
            }
        } else if (strcmp(nomesCampos[j], "nomeLinha") == 0) {
            if (valoresStrings[j][0] == '\0') {
                if (reg->nomeLinha != NULL) return 0;
            } else {
                if (reg->nomeLinha == NULL ||
                    strcmp(reg->nomeLinha, valoresStrings[j]) != 0) return 0;
            }
        }
    }
    return 1;
}

/*
 * Funcionalidade 6 - SELECT com WHERE.
 *
 * Le n buscas da entrada. Para cada busca, le os filtros (m pares de
 * nomeCampo/valorCampo) e imprime todos os registros do arquivo de dados que
 * satisfazem TODOS os filtros ao mesmo tempo.
 *
 * Se o filtro for so o campo codEstacao (e nenhum outro), usa o arquivo de
 * indice primario pra achar o RRN com busca binaria e ir direto no registro
 * (busca indexada). Para qualquer outra combinacao de campos, percorre o
 * arquivo de dados do inicio ao fim conferindo cada registro nao removido
 * contra os filtros (busca sequencial).
 *
 * Se nenhum registro satisfizer os filtros de uma busca, imprime
 * "Registro inexistente.". Registros marcados como removidos nunca aparecem
 * no resultado.
 */
void buscaIndexada(char *nomeArquivoDados, char *nomeArquivoIndice) {
    FILE *fDados = fopen(nomeArquivoDados, "rb");
    if (!fDados) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // confere se o arquivo de dados ficou consistente da ultima vez que foi usado
    char statusDados;
    fread(&statusDados, sizeof(char), 1, fDados);
    if (statusDados == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    FILE *fIndice = fopen(nomeArquivoIndice, "rb");
    if (!fIndice) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    // mesma verificacao de consistencia, agora pro arquivo de indice
    char statusIndice;
    fread(&statusIndice, sizeof(char), 1, fIndice);
    if (statusIndice == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        fclose(fIndice);
        return;
    }

    // descobre quantos registros existem no indice a partir do tamanho do arquivo
    fseek(fIndice, 0, SEEK_END);
    long tamIndice = ftell(fIndice);
    int nRegIndice = (int)((tamIndice - TAM_CABECALHO_INDICE) / TAM_REGISTRO_INDICE);

    int nBuscas;
    scanf("%d", &nBuscas);

    for (int i = 0; i < nBuscas; i++) {
        int mFiltros;
        scanf("%d", &mFiltros);

        // armazena os filtros dessa busca: nome do campo, valor (string ou int)
        char nomesCampos[10][50];
        char valoresStrings[10][100];
        int  valoresInts[10];
        int  buscaPorCodEstacao = 0;
        int  codEstacaoBuscado  = -1;

        for (int j = 0; j < mFiltros; j++) {
            valoresStrings[j][0] = '\0';
            valoresInts[j]       = VALOR_NULO_INT;

            scanf("%s", nomesCampos[j]);

            if (strcmp(nomesCampos[j], "nomeEstacao") == 0 ||
                strcmp(nomesCampos[j], "nomeLinha")   == 0) {
                // campos string podem vir entre aspas ou como NULO
                ScanQuoteString(valoresStrings[j]);
            } else {
                char temp[50];
                scanf("%s", temp);
                if (strcmp(temp, "NULO") == 0) valoresInts[j] = VALOR_NULO_INT;
                else                           valoresInts[j] = atoi(temp);

                if (strcmp(nomesCampos[j], "codEstacao") == 0) {
                    codEstacaoBuscado = valoresInts[j];
                }
            }
        }

        // so vale a pena usar o indice quando o codEstacao e o UNICO criterio
        // de busca. Se tiver mais filtros junto, precisa varrer o arquivo
        // inteiro mesmo, ai nao tem vantagem usar o indice
        if (mFiltros == 1 && strcmp(nomesCampos[0], "codEstacao") == 0) {
            buscaPorCodEstacao = 1;
        }

        int encontrouAlgum = 0;

        if (buscaPorCodEstacao) {
            // busca indexada: acha o RRN no indice (binaria) e le so esse registro
            int rrn = busca_binaria_indice(fIndice, codEstacaoBuscado, nRegIndice);

            if (rrn >= 0) {
                Registro reg;
                if (ler_registro_por_rrn(fDados, rrn, &reg) && reg.removido != '1') {
                    imprimir_registro(&reg);
                    encontrouAlgum = 1;
                }
                if (reg.nomeEstacao) free(reg.nomeEstacao);
                if (reg.nomeLinha)   free(reg.nomeLinha);
            }
        } else {
            // busca sequencial: percorre todo o arquivo de dados registro a registro
            fseek(fDados, TAM_CABECALHO, SEEK_SET);

            Registro reg;
            while (ler_registro_binario(fDados, &reg)) {
                if (reg.removido == '1') {
                    if (reg.nomeEstacao) free(reg.nomeEstacao);
                    if (reg.nomeLinha)   free(reg.nomeLinha);
                    continue;
                }

                if (satisfaz_filtros(&reg, mFiltros, nomesCampos,
                                     valoresStrings, valoresInts)) {
                    imprimir_registro(&reg);
                    encontrouAlgum = 1;
                }

                if (reg.nomeEstacao) free(reg.nomeEstacao);
                if (reg.nomeLinha)   free(reg.nomeLinha);
            }
        }

        if (!encontrouAlgum) {
            printf("Registro inexistente.\n");
        }

        // separa a saida de cada busca com uma linha em branco, exceto a ultima
        if (i < nBuscas - 1) {
            printf("\n");
        }
    }

    fclose(fDados);
    fclose(fIndice);
}