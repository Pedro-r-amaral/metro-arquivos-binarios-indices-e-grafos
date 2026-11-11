#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "registros.h"
#include "io_binario.h"
#include "impressao.h"
#include "fornecidas.h"

/*
 * Le os 17 bytes do cabecalho do arquivo de dados e guarda em memoria.
 * Precisa ser feito assim (campo a campo) porque o cabecalho tem campos de
 * tamanho diferente (1 char + 4 ints).
 */
static void ler_cabecalho(FILE *fp, Cabecalho *cab) {
    fseek(fp, 0, SEEK_SET);
    fread(&cab->status,          sizeof(char), 1, fp);
    fread(&cab->topo,            sizeof(int),  1, fp);
    fread(&cab->proxRRN,         sizeof(int),  1, fp);
    fread(&cab->nroEstacoes,     sizeof(int),  1, fp);
    fread(&cab->nroParesEstacao, sizeof(int),  1, fp);
}

/*
 * Remove a entrada de codEstacao do arquivo de indice primario.
 *
 * Como o indice e mantido sempre ordenado por codEstacao, primeiro acha a
 * posicao da chave com busca binaria. Depois, desloca todos os registros que
 * vinham depois dela uma posicao pra tras (sobrescrevendo a entrada removida)
 * e trunca o arquivo, removendo o ultimo registro que ficou duplicado.
 *
 * Se o codEstacao nao for encontrado no indice, nao faz nada.
 */
static void remover_do_indice(FILE *fIndice, int codEstacao) {
    // calcula quantos registros tem no indice pelo tamanho do arquivo
    fseek(fIndice, 0, SEEK_END);
    long tam = ftell(fIndice);
    int nReg = (int)((tam - TAM_CABECALHO_INDICE) / TAM_REGISTRO_INDICE);

    // busca binaria pra achar a posicao do registro com esse codEstacao
    int esq = 0, dir = nReg - 1, pos = -1;
    while (esq <= dir) {
        int meio = (esq + dir) / 2;
        fseek(fIndice, TAM_CABECALHO_INDICE + meio * TAM_REGISTRO_INDICE, SEEK_SET);
        RegistroIndice ri;
        fread(&ri.codEstacao, sizeof(int32_t), 1, fIndice);
        fread(&ri.rrn,        sizeof(int32_t), 1, fIndice);
        if (ri.codEstacao == codEstacao) { pos = meio; break; }
        if (ri.codEstacao < codEstacao)  esq = meio + 1;
        else                             dir = meio - 1;
    }

    if (pos == -1) return; // codEstacao nao esta no indice, nada a fazer

    // puxa cada registro depois de "pos" uma posicao pra tras, mantendo a ordem
    for (int i = pos + 1; i < nReg; i++) {
        fseek(fIndice, TAM_CABECALHO_INDICE + i * TAM_REGISTRO_INDICE, SEEK_SET);
        RegistroIndice ri;
        fread(&ri.codEstacao, sizeof(int32_t), 1, fIndice);
        fread(&ri.rrn,        sizeof(int32_t), 1, fIndice);

        fseek(fIndice, TAM_CABECALHO_INDICE + (i - 1) * TAM_REGISTRO_INDICE, SEEK_SET);
        fwrite(&ri.codEstacao, sizeof(int32_t), 1, fIndice);
        fwrite(&ri.rrn,        sizeof(int32_t), 1, fIndice);
    }

    // o ultimo registro agora esta repetido (foi copiado pra posicao anterior),
    // entao trunca o arquivo removendo esse espaco sobrando
    fflush(fIndice);
    long novoTam = TAM_CABECALHO_INDICE + (long)(nReg - 1) * TAM_REGISTRO_INDICE;
    ftruncate(fileno(fIndice), novoTam);
}

/*
 * Confere se um registro atende todos os filtros de busca passados pelo
 * usuario (mesma logica usada na funcionalidade 6: e um "AND" entre os
 * filtros, e valor vazio em campo string significa busca por NULO).
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
            if (valoresStrings[j][0] == '\0') {
                if (reg->nomeEstacao != NULL) return 0;
            } else {
                if (reg->nomeEstacao == NULL) return 0;
                if (strcmp(reg->nomeEstacao, valoresStrings[j]) != 0) return 0;
            }
        } else if (strcmp(nomesCampos[j], "nomeLinha") == 0) {
            if (valoresStrings[j][0] == '\0') {
                if (reg->nomeLinha != NULL) return 0;
            } else {
                if (reg->nomeLinha == NULL) return 0;
                if (strcmp(reg->nomeLinha, valoresStrings[j]) != 0) return 0;
            }
        }
    }
    return 1;
}

/*
 * Funcionalidade 7 - DELETE com WHERE.
 *
 * Faz a remocao logica de registros, usando a abordagem de pilha de
 * registros removidos vista em aula:
 *  - o registro nao e apagado fisicamente, so marcado como removido (byte
 *    "removido" = '1')
 *  - o campo "proximo" do registro removido passa a apontar para o RRN que
 *    era o topo da pilha antes
 *  - o cabecalho passa a apontar (cab.topo) para o RRN do registro que
 *    acabou de ser removido, virando o novo topo da pilha
 *
 * Isso forma uma lista encadeada de espacos livres que a funcionalidade 8
 * (insercao) pode reaproveitar depois.
 *
 * A funcao roda n vezes seguidas (uma remocao por iteracao). Em cada
 * iteracao, le os criterios de busca e percorre o arquivo de dados removendo
 * TODOS os registros nao removidos que satisfazem esses criterios. Se
 * nenhum registro satisfizer, simplesmente nao remove nada naquela iteracao
 * e segue para a proxima (sem dar erro).
 *
 * Toda vez que um registro e removido, sua entrada correspondente tambem e
 * apagada do arquivo de indice primario.
 *
 * Antes de comecar, marca os dois arquivos (dados e indice) como
 * inconsistentes (status '0'), e so volta pra '1' depois de terminar todas
 * as remocoes - assim, se o programa for interrompido no meio, da pra saber
 * que o arquivo ficou incompleto.
 */
void remocaoLogica(char *nomeArquivoDados, char *nomeArquivoIndice) {
    FILE *fDados = fopen(nomeArquivoDados, "r+b");
    if (!fDados) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // confere se o arquivo de dados estava consistente antes de abrir pra escrita
    char statusDados;
    fread(&statusDados, sizeof(char), 1, fDados);
    if (statusDados == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    FILE *fIndice = fopen(nomeArquivoIndice, "r+b");
    if (!fIndice) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    // mesma verificacao de consistencia pro indice
    char statusIndice;
    fread(&statusIndice, sizeof(char), 1, fIndice);
    if (statusIndice == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        fclose(fIndice);
        return;
    }

    // marca os dois arquivos como inconsistentes enquanto a operacao acontece
    char inconsistente = '0';
    fseek(fDados,  0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, fDados);
    fseek(fIndice, 0, SEEK_SET);
    fwrite(&inconsistente, sizeof(char), 1, fIndice);

    // carrega o cabecalho do arquivo de dados pra memoria (precisa do topo da
    // pilha pra encadear os novos removidos)
    fseek(fDados, 0, SEEK_SET);
    Cabecalho cab;
    ler_cabecalho(fDados, &cab);

    int nRemocoes;
    scanf("%d", &nRemocoes);

    for (int i = 0; i < nRemocoes; i++) {
        int mFiltros;
        scanf("%d", &mFiltros);

        // le os criterios de busca (nomeCampo + valorCampo) dessa remocao
        char nomesCampos[10][50];
        char valoresStrings[10][100];
        int  valoresInts[10];

        for (int j = 0; j < mFiltros; j++) {
            valoresStrings[j][0] = '\0';
            valoresInts[j]       = VALOR_NULO_INT;
            scanf("%s", nomesCampos[j]);
            if (strcmp(nomesCampos[j], "nomeEstacao") == 0 ||
                strcmp(nomesCampos[j], "nomeLinha")   == 0) {
                ScanQuoteString(valoresStrings[j]);
            } else {
                char temp[50];
                scanf("%s", temp);
                if (strcmp(temp, "NULO") == 0) valoresInts[j] = VALOR_NULO_INT;
                else                           valoresInts[j] = atoi(temp);
            }
        }

        // percorre o arquivo de dados do inicio, registro por registro,
        // procurando quem satisfaz os criterios pra remover
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

            if (satisfaz_filtros(&reg, mFiltros, nomesCampos, valoresStrings, valoresInts)) {
                int codParaRemover = reg.codEstacao;

                // Checar se a estação ainda sobrevive no arquivo (em outra linha, por exemplo)
                int estacao_ainda_existe = 0;
                long posAtual = ftell(fDados);
                fseek(fDados, TAM_CABECALHO, SEEK_SET);
                Registro aux;
                while (ler_registro_binario(fDados, &aux)) {
                    // Ignora o próprio registro que estamos deletando e os já removidos
                    if (aux.removido == '0' && aux.codEstacao != reg.codEstacao) {
                        if (aux.nomeEstacao && reg.nomeEstacao && strcmp(aux.nomeEstacao, reg.nomeEstacao) == 0) {
                            estacao_ainda_existe = 1;
                        }
                    }
                    if (aux.nomeEstacao) free(aux.nomeEstacao);
                    if (aux.nomeLinha) free(aux.nomeLinha);
                    if (estacao_ainda_existe) break;
                }

                // Se a estação não existe mais em nenhuma outra linha, aí sim ela deixa de ser única!
                if (!estacao_ainda_existe) {
                    cab.nroEstacoes--;
                }

                // Já o par é exclusivo da conexão, então sempre apagamos o par
                if (reg.codProxEstacao != VALOR_NULO_INT && reg.codEstacao != reg.codProxEstacao) {
                    cab.nroParesEstacao--;
                }

                // marca o registro como removido e encadeia ele no topo da pilha:
                // o campo "proximo" passa a apontar pro antigo topo, e o topo
                // do cabecalho passa a ser esse rrn
                long offset = (long)TAM_CABECALHO + (long)rrnAtual * TAM_REGISTRO_DADOS;
                fseek(fDados, offset, SEEK_SET);

                char marcaRemovido = '1';
                fwrite(&marcaRemovido, sizeof(char), 1, fDados);
                fwrite(&cab.topo,      sizeof(int),  1, fDados);

                cab.topo = rrnAtual;

                // volta o ponteiro do arquivo pra logo depois deste registro,
                // pra continuar a varredura normalmente
                fseek(fDados, offset + TAM_REGISTRO_DADOS, SEEK_SET);

                // tira a chave desse registro do indice primario
                remover_do_indice(fIndice, codParaRemover);
            }

            if (reg.nomeEstacao) free(reg.nomeEstacao);
            if (reg.nomeLinha)   free(reg.nomeLinha);
            rrnAtual++;
        }
    }

    // grava o cabecalho atualizado (com o novo topo da pilha) e volta o
    // status dos dois arquivos pra consistente
    cab.status = '1';
    escreverCabecalho(fDados, &cab);

    char consistente = '1';
    fseek(fIndice, 0, SEEK_SET);
    fwrite(&consistente, sizeof(char), 1, fIndice);

    fclose(fDados);
    fclose(fIndice);

    // exibe os dois arquivos atualizados conforme pedido na especificacao
    BinarioNaTela(nomeArquivoDados);
    BinarioNaTela(nomeArquivoIndice);
}