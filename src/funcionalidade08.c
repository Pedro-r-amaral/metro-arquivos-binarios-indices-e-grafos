#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"
#include "fornecidas.h"

//Função de comparação isolada no escopo global do arquivo
static int comparar_indice(const void *a, const void *b) {
    RegistroIndice *idxA = (RegistroIndice *)a;
    RegistroIndice *idxB = (RegistroIndice *)b;
    if (idxA->codEstacao < idxB->codEstacao) return -1;
    if (idxA->codEstacao > idxB->codEstacao) return 1;
    return 0;
}

//Função auxiliar modificada para chamar o qsort global
static void inserir_no_indice(FILE *fIndice, int32_t codEstacao, int32_t rrn) {
    fseek(fIndice, 0, SEEK_END);
    long tam = ftell(fIndice);
    int nReg = (int)((tam - TAM_CABECALHO_INDICE) / TAM_REGISTRO_INDICE);

    RegistroIndice *lista = malloc(sizeof(RegistroIndice) * (nReg + 1));
    
    fseek(fIndice, TAM_CABECALHO_INDICE, SEEK_SET);
    for (int i = 0; i < nReg; i++) {
        fread(&lista[i].codEstacao, sizeof(int32_t), 1, fIndice);
        fread(&lista[i].rrn,        sizeof(int32_t), 1, fIndice);
    }

    lista[nReg].codEstacao = codEstacao;
    lista[nReg].rrn = rrn;
    nReg++;

    qsort(lista, nReg, sizeof(RegistroIndice), comparar_indice);

    fseek(fIndice, TAM_CABECALHO_INDICE, SEEK_SET);
    for (int i = 0; i < nReg; i++) {
        fwrite(&lista[i].codEstacao, sizeof(int32_t), 1, fIndice);
        fwrite(&lista[i].rrn,        sizeof(int32_t), 1, fIndice);
    }

    free(lista);
}

//Funcionalidade 8: Inserção de novos registros reaproveitando espaço da pilha
void insercaoDados(char *nomeArquivoDados, char *nomeArquivoIndice) {
    //abre o arquivo de dados para leitura e escrita binaria
    FILE *fDados = fopen(nomeArquivoDados, "r+b");
    if (!fDados) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //verifica consistencia do arquivo de dados pelo status
    char statusDados;
    fread(&statusDados, sizeof(char), 1, fDados);
    if (statusDados == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    //abre o arquivo de indice para leitura e escrita
    FILE *fIndice = fopen(nomeArquivoIndice, "r+b");
    if (!fIndice) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return;
    }

    //verifica consistencia do arquivo de indice
    char statusIndice;
    fread(&statusIndice, sizeof(char), 1, fIndice);
    if (statusIndice == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        fclose(fIndice);
        return;
    }

    //setar status como inconsistente ('0') durante a operação
    char inconsistente = '0';
    fseek(fDados, 0, SEEK_SET);  fwrite(&inconsistente, sizeof(char), 1, fDados);
    fseek(fIndice, 0, SEEK_SET); fwrite(&inconsistente, sizeof(char), 1, fIndice);

    //carregar o cabeçalho de dados para a memória
    fseek(fDados, 0, SEEK_SET);
    Cabecalho cab;
    fseek(fDados, 0, SEEK_SET);
    fread(&cab.status,          sizeof(char), 1, fDados);
    fread(&cab.topo,            sizeof(int),  1, fDados); //topo da pilha de removidos
    fread(&cab.proxRRN,         sizeof(int),  1, fDados); //proximo RRN livre (fim do arquivo)
    fread(&cab.nroEstacoes,     sizeof(int),  1, fDados);
    fread(&cab.nroParesEstacao, sizeof(int),  1, fDados);

    int nInsercoes;
    scanf("%d", &nInsercoes); //quantidade de registros a inserir

    for (int i = 0; i < nInsercoes; i++) {
        Registro reg;
        reg.removido = '0'; //registro ativo
        reg.proximo = -1; //nao aponta para ninguém (nao esta na pilha)

        // 1.ler os campos da entrada padrão
        char temp[100];
        
        scanf("%s", temp);
        reg.codEstacao = (strcmp(temp, "NULO") == 0) ? VALOR_NULO_INT : atoi(temp);

        ScanQuoteString(temp);
        reg.tamNomeEstacao = strlen(temp);
        if (reg.tamNomeEstacao > 0) {
            reg.nomeEstacao = malloc(reg.tamNomeEstacao + 1);
            strcpy(reg.nomeEstacao, temp);
        } else {
            reg.nomeEstacao = NULL;
        }

        scanf("%s", temp);
        reg.codLinha = (strcmp(temp, "NULO") == 0) ? VALOR_NULO_INT : atoi(temp);

        ScanQuoteString(temp);
        reg.tamNomeLinha = strlen(temp);
        if (reg.tamNomeLinha > 0) {
            reg.nomeLinha = malloc(reg.tamNomeLinha + 1);
            strcpy(reg.nomeLinha, temp);
        } else {
            reg.nomeLinha = NULL;
        }

        scanf("%s", temp);
        reg.codProxEstacao = (strcmp(temp, "NULO") == 0) ? VALOR_NULO_INT : atoi(temp);

        scanf("%s", temp);
        reg.distProxEstacao = (strcmp(temp, "NULO") == 0) ? VALOR_NULO_INT : atoi(temp);

        scanf("%s", temp);
        reg.codLinhaIntegra = (strcmp(temp, "NULO") == 0) ? VALOR_NULO_INT : atoi(temp);

        scanf("%s", temp);
        reg.codEstIntegra = (strcmp(temp, "NULO") == 0) ? VALOR_NULO_INT : atoi(temp);

        //VERIFICAÇÃO EXATA DE UNICIDADE (SÓ NOME PARA ESTAÇÕES, PERMITE SELF-LOOPS)
        // define se eh uma nova estacao ou um novo par de estacoes (nao direcionado) para atualizar os contadores do cabecalho
        int nova_estacao = 0;
        if (reg.codEstacao != VALOR_NULO_INT) {
            nova_estacao = 1;
        }

        int novo_par = 0;
        if (reg.codEstacao != VALOR_NULO_INT && reg.codProxEstacao != VALOR_NULO_INT) {
            novo_par = 1;
        }

        if (nova_estacao || novo_par) {
            fseek(fDados, TAM_CABECALHO, SEEK_SET);
            Registro aux;
            while (ler_registro_binario(fDados, &aux)) {
                if (aux.removido == '0') {
                    
                    //checa unicidade da estação pelo NOME
                    if (nova_estacao) {
                        if (reg.nomeEstacao != NULL && aux.nomeEstacao != NULL) {
                            if (strcmp(aux.nomeEstacao, reg.nomeEstacao) == 0) {
                                nova_estacao = 0; // Nome já existe
                            }
                        } else if (reg.nomeEstacao == NULL && aux.codEstacao == reg.codEstacao) {
                            //fallback seguro caso insiram uma estação sem nome
                            nova_estacao = 0;
                        }
                    }

                    //checa unicidade do par (nao direcionado)
                    if (novo_par && aux.codEstacao != VALOR_NULO_INT && aux.codProxEstacao != VALOR_NULO_INT) {
                        if ((aux.codEstacao == reg.codEstacao && aux.codProxEstacao == reg.codProxEstacao) ||
                            (aux.codEstacao == reg.codProxEstacao && aux.codProxEstacao == reg.codEstacao)) {
                            novo_par = 0; //par ja existe
                        }
                    }
                }
                
                if (aux.nomeEstacao) free(aux.nomeEstacao);
                if (aux.nomeLinha) free(aux.nomeLinha);

                if (!nova_estacao && !novo_par) break; 
            }
        }

        if (nova_estacao) cab.nroEstacoes++;
        if (novo_par) cab.nroParesEstacao++;

        int rrnDestino;

        // 2.logica da Pilha
        if (cab.topo != -1) {
            //reutiliza posição do topo da pilha
            rrnDestino = cab.topo;
            long offset = (long)TAM_CABECALHO + (long)rrnDestino * TAM_REGISTRO_DADOS;
            fseek(fDados, offset + 1, SEEK_SET); //pula campo 'removido'
            
            int proximoTopo;
            fread(&proximoTopo, sizeof(int), 1, fDados); //le encadeamento da pilha
            cab.topo = proximoTopo; //atualiza topo 
        } else {
            //nao ha espaço reaproveitavel, usa final do arquivo
            rrnDestino = cab.proxRRN;
            cab.proxRRN++;
        }

        // 3.gravar
        long offsetGrava = (long)TAM_CABECALHO + (long)rrnDestino * TAM_REGISTRO_DADOS;
        fseek(fDados, offsetGrava, SEEK_SET);
        escreverRegistro(fDados, &reg);

        // 4.indice
        if (reg.codEstacao != VALOR_NULO_INT) {
            inserir_no_indice(fIndice, reg.codEstacao, rrnDestino);
        }
        // libera memoria alocada para o registro atual
        if (reg.nomeEstacao) free(reg.nomeEstacao);
        if (reg.nomeLinha)   free(reg.nomeLinha);
    }

    // 5.fechar
    cab.status = '1';
    escreverCabecalho(fDados, &cab);

    char consistente = '1';
    fseek(fIndice, 0, SEEK_SET);
    fwrite(&consistente, sizeof(char), 1, fIndice);

    fclose(fDados);
    fclose(fIndice);

    BinarioNaTela(nomeArquivoDados);
    BinarioNaTela(nomeArquivoIndice);
}