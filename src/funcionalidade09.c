#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"
#include "fornecidas.h"

// Função de comparação isolada no escopo global do arquivo
static int comparar_atualizacao(const void *a, const void *b) {
    RegistroIndice *idxA = (RegistroIndice *)a;
    RegistroIndice *idxB = (RegistroIndice *)b;
    if (idxA->codEstacao < idxB->codEstacao) return -1;
    if (idxA->codEstacao > idxB->codEstacao) return 1;
    return 0;
}

// Função auxiliar modificada para usar a comparação global
static void atualizar_chave_indice(FILE *fIndice, int32_t codAntigo, int32_t codNovo, int32_t rrn) {
    fseek(fIndice, 0, SEEK_END);
    long tam = ftell(fIndice);
    int nReg = (int)((tam - TAM_CABECALHO_INDICE) / TAM_REGISTRO_INDICE);

    if (nReg <= 0) return;

    RegistroIndice *lista = malloc(sizeof(RegistroIndice) * nReg);
    fseek(fIndice, TAM_CABECALHO_INDICE, SEEK_SET);
    
    int encontradoIdx = -1;
    for (int i = 0; i < nReg; i++) {
        fread(&lista[i].codEstacao, sizeof(int32_t), 1, fIndice);
        fread(&lista[i].rrn,        sizeof(int32_t), 1, fIndice);
        if (lista[i].codEstacao == codAntigo && lista[i].rrn == rrn) {
            encontradoIdx = i;
        }
    }

    if (encontradoIdx != -1) {
        lista[encontradoIdx].codEstacao = codNovo;
    }

    qsort(lista, nReg, sizeof(RegistroIndice), comparar_atualizacao);

    fseek(fIndice, TAM_CABECALHO_INDICE, SEEK_SET);
    for (int i = 0; i < nReg; i++) {
        fwrite(&lista[i].codEstacao, sizeof(int32_t), 1, fIndice);
        fwrite(&lista[i].rrn,        sizeof(int32_t), 1, fIndice);
    }

    free(lista);
}

// Aplica as modificações lidas da entrada nos campos correspondentes da struct Registro
static void aplicar_atualizacao_no_registro(Registro *reg, int pCampos, char nomesCamposA[][50], char valoresStringsA[][100], int valoresIntsA[], int *mudouCodEstacao, int *novoCodEstacao) {
    for (int i = 0; i < pCampos; i++) {
        if (strcmp(nomesCamposA[i], "codEstacao") == 0) {
            *mudouCodEstacao = 1;
            *novoCodEstacao = valoresIntsA[i];
            reg->codEstacao = valoresIntsA[i];
        } else if (strcmp(nomesCamposA[i], "codLinha") == 0) {
            reg->codLinha = valoresIntsA[i];
        } else if (strcmp(nomesCamposA[i], "codProxEstacao") == 0) {
            reg->codProxEstacao = valoresIntsA[i];
        } else if (strcmp(nomesCamposA[i], "distProxEstacao") == 0) {
            reg->distProxEstacao = valoresIntsA[i];
        } else if (strcmp(nomesCamposA[i], "codLinhaIntegra") == 0) {
            reg->codLinhaIntegra = valoresIntsA[i];
        } else if (strcmp(nomesCamposA[i], "codEstIntegra") == 0) {
            reg->codEstIntegra = valoresIntsA[i];
        } else if (strcmp(nomesCamposA[i], "nomeEstacao") == 0) {
            if (reg->nomeEstacao) free(reg->nomeEstacao);
            if (valoresStringsA[i][0] == '\0') {
                reg->nomeEstacao = NULL;
                reg->tamNomeEstacao = 0;
            } else {
                reg->tamNomeEstacao = strlen(valoresStringsA[i]);
                reg->nomeEstacao = malloc(reg->tamNomeEstacao + 1);
                strcpy(reg->nomeEstacao, valoresStringsA[i]);
            }
        } else if (strcmp(nomesCamposA[i], "nomeLinha") == 0) {
            if (reg->nomeLinha) free(reg->nomeLinha);
            if (valoresStringsA[i][0] == '\0') {
                reg->nomeLinha = NULL;
                reg->tamNomeLinha = 0;
            } else {
                reg->tamNomeLinha = strlen(valoresStringsA[i]);
                reg->nomeLinha = malloc(reg->tamNomeLinha + 1);
                strcpy(reg->nomeLinha, valoresStringsA[i]);
            }
        }
    }
}

// Verifica se um registro em memória bate com as especificações do filtro de busca
static int atende_filtros_busca(Registro *reg, int mFiltros, char nomesCamposB[][50], char valoresStringsB[][100], int valoresIntsB[]) {
    for (int j = 0; j < mFiltros; j++) {
        if (strcmp(nomesCamposB[j], "codEstacao") == 0) {
            if (reg->codEstacao != valoresIntsB[j]) return 0;
        } else if (strcmp(nomesCamposB[j], "codLinha") == 0) {
            if (reg->codLinha != valoresIntsB[j]) return 0;
        } else if (strcmp(nomesCamposB[j], "codProxEstacao") == 0) {
            if (reg->codProxEstacao != valoresIntsB[j]) return 0;
        } else if (strcmp(nomesCamposB[j], "distProxEstacao") == 0) {
            if (reg->distProxEstacao != valoresIntsB[j]) return 0;
        } else if (strcmp(nomesCamposB[j], "codLinhaIntegra") == 0) {
            if (reg->codLinhaIntegra != valoresIntsB[j]) return 0;
        } else if (strcmp(nomesCamposB[j], "codEstIntegra") == 0) {
            if (reg->codEstIntegra != valoresIntsB[j]) return 0;
        } else if (strcmp(nomesCamposB[j], "nomeEstacao") == 0) {
            if (valoresStringsB[j][0] == '\0') {
                if (reg->nomeEstacao != NULL) return 0;
            } else {
                if (reg->nomeEstacao == NULL) return 0; 
                if (strcmp(reg->nomeEstacao, valoresStringsB[j]) != 0) return 0;
            }
        } else if (strcmp(nomesCamposB[j], "nomeLinha") == 0) {
            if (valoresStringsB[j][0] == '\0') {
                if (reg->nomeLinha != NULL) return 0;
            } else {
                if (reg->nomeLinha == NULL) return 0;
                if (strcmp(reg->nomeLinha, valoresStringsB[j]) != 0) return 0;
            }
        }
    }
    return 1;
}

// Funcionalidade 9: Atualização direta em disco de registros com tamanho fixo
void atualizacaoDados(char *nomeArquivoDados, char *nomeArquivoIndice) {
    //abre o arquivo de dados para leitura/escrita
    FILE *fDados = fopen(nomeArquivoDados, "r+b");
    if (!fDados) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //verifica consistência do arquivo de dados
    char statusDados;
    fread(&statusDados, sizeof(char), 1, fDados);
    if (statusDados == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    // abre o arquivo de indice
    FILE *fIndice = fopen(nomeArquivoIndice, "r+b");
    if (!fIndice) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    //verifica consistência do indice
    char statusIndice;
    fread(&statusIndice, sizeof(char), 1, fIndice);
    if (statusIndice == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        fclose(fIndice);
        return;
    }

    //setar status de inconsistência '0' temporário
    char inconsistente = '0';
    fseek(fDados, 0, SEEK_SET);  fwrite(&inconsistente, sizeof(char), 1, fDados);
    fseek(fIndice, 0, SEEK_SET); fwrite(&inconsistente, sizeof(char), 1, fIndice);

    int nAtualizacoes;
    scanf("%d", &nAtualizacoes); //quantidade de comandos de atualização

    for (int i = 0; i < nAtualizacoes; i++) {
        int mFiltros;
        scanf("%d", &mFiltros); //numero de condições WHERE

        char nomesCamposB[10][50];
        char valoresStringsB[10][100];
        int valoresIntsB[10];

        //lendo os criterios de BUSCA (WHERE)
        for (int j = 0; j < mFiltros; j++) {
            valoresStringsB[j][0] = '\0';
            valoresIntsB[j] = VALOR_NULO_INT;
            scanf("%s", nomesCamposB[j]);
            if (strcmp(nomesCamposB[j], "nomeEstacao") == 0 || strcmp(nomesCamposB[j], "nomeLinha") == 0) {
                ScanQuoteString(valoresStringsB[j]);
            } else {
                char temp[50];
                scanf("%s", temp);
                if (strcmp(temp, "NULO") == 0) valoresIntsB[j] = VALOR_NULO_INT;
                else                           valoresIntsB[j] = atoi(temp);
            }
        }

        int pCampos;
        scanf("%d", &pCampos);

        char nomesCamposA[10][50];
        char valoresStringsA[10][100];
        int valoresIntsA[10];

        //lendo as ATUALIZACOES (SET)
        for (int j = 0; j < pCampos; j++) {
            valoresStringsA[j][0] = '\0';
            valoresIntsA[j] = VALOR_NULO_INT;
            scanf("%s", nomesCamposA[j]);
            if (strcmp(nomesCamposA[j], "nomeEstacao") == 0 || strcmp(nomesCamposA[j], "nomeLinha") == 0) {
                ScanQuoteString(valoresStringsA[j]);
            } else {
                char temp[50];
                scanf("%s", temp);
                if (strcmp(temp, "NULO") == 0) valoresIntsA[j] = VALOR_NULO_INT;
                else                           valoresIntsA[j] = atoi(temp);
            }
        }

        //varredura sequencial para encontrar todos os registros compativeis
        fseek(fDados, TAM_CABECALHO, SEEK_SET);
        int rrnAtual = 0;
        Registro reg;

        while (ler_registro_binario(fDados, &reg)) {
            if (reg.removido == '1') {
                if (reg.nomeEstacao) free(reg.nomeEstacao);
                if (reg.nomeLinha)   free(reg.nomeLinha);
                rrnAtual++;
                continue;
            }

            if (atende_filtros_busca(&reg, mFiltros, nomesCamposB, valoresStringsB, valoresIntsB)) {
                int32_t codEstacaoAntigo = reg.codEstacao;
                int mudouCodEstacao = 0;
                int novoCodEstacao = -1;

                //modifica o registro em memoria
                aplicar_atualizacao_no_registro(&reg, pCampos, nomesCamposA, valoresStringsA, valoresIntsA, &mudouCodEstacao, &novoCodEstacao);

                //retorna o ponteiro do arquivo ao início do registro fisico atualizado e o sobrescreve
                long offsetGrava = (long)TAM_CABECALHO + (long)rrnAtual * TAM_REGISTRO_DADOS;
                fseek(fDados, offsetGrava, SEEK_SET);
                escreverRegistro(fDados, &reg);

                fseek(fDados, offsetGrava + TAM_REGISTRO_DADOS, SEEK_SET);

                //se alterou a chave primaria, ajusta o índice correspondente em tempo real
                if (mudouCodEstacao && codEstacaoAntigo != VALOR_NULO_INT) {
                    atualizar_chave_indice(fIndice, codEstacaoAntigo, novoCodEstacao, rrnAtual);
                }
            }

            if (reg.nomeEstacao) free(reg.nomeEstacao);
            if (reg.nomeLinha)   free(reg.nomeLinha);
            rrnAtual++;
        }
    }

    //voltar status para consistente '1' nos cabeçalhos
    fseek(fDados, 0, SEEK_SET);
    char consistente = '1';
    fwrite(&consistente, sizeof(char), 1, fDados);

    fseek(fIndice, 0, SEEK_SET);
    fwrite(&consistente, sizeof(char), 1, fIndice);

    fclose(fDados);
    fclose(fIndice);

    BinarioNaTela(nomeArquivoDados);
    BinarioNaTela(nomeArquivoIndice);
}