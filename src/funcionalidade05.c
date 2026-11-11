#include "registros.h"
#include "io_binario.h"
#include <stdlib.h>
#include <string.h>

//compara para o qsort ordenar por codEstacao de forma crescente 
static int compararIndices(const void *a, const void *b) {
    RegistroIndice *idxA = (RegistroIndice *)a;
    RegistroIndice *idxB = (RegistroIndice *)b;
    if (idxA->codEstacao < idxB->codEstacao) return -1;
    if (idxA->codEstacao > idxB->codEstacao) return 1;
    return 0;
}

//abrir arquivo de dados para leitura
int criarIndicePrimario(const char *nomeArquivoDados, const char *nomeArquivoIndice) {
    FILE *fDados = fopen(nomeArquivoDados, "rb");
    if (!fDados) {
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }

    //verificacao do status do arquivo de dados (consistencia)
    char statusDados;
    if (fread(&statusDados, sizeof(char), 1, fDados) != 1 || statusDados == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return 0;
    }

    //abrir/criar arquivo de indice para escrita binaria
    FILE *fIndice = fopen(nomeArquivoIndice, "wb");
    if (!fIndice) {
        printf("Falha no processamento do arquivo.\n");
        fclose(fDados);
        return 0;
    }

    //escreve status '0' no cabecalho do indice indicando que esta sendo processado
    CabecalhoIndice cabIdx = {'0'};
    fwrite(&cabIdx.status, sizeof(char), 1, fIndice);

    //pula o restante do cabecalho do arquivo de dados para começar a ler os registros
    fseek(fDados, 16, SEEK_CUR);

    //estruturas dinamicas para armazenar os indices em memoria antes de ordenar
    RegistroIndice *listaIndices = NULL;
    int nRegistros = 0;
    int rrnAtual = 0;

    Registro reg;
    //varre arquivo de dados registro por registro 
    while (ler_registro_binario(fDados, &reg)) {
        //registros removidos logicamente vao ser ignorados no indice primario
        if (reg.removido == '0') {
            listaIndices = realloc(listaIndices, sizeof(RegistroIndice) * (nRegistros + 1));
            listaIndices[nRegistros].codEstacao = reg.codEstacao;
            listaIndices[nRegistros].rrn = rrnAtual;
            nRegistros++;
        }
        
        //liberar strings alocadas pela funcao fornecida de leitura 
        if (reg.nomeEstacao) free(reg.nomeEstacao);
        if (reg.nomeLinha) free(reg.nomeLinha);
        
        //incrementa o RRN (posicao logica do registro no arquivo)
        rrnAtual++;
    }
    fclose(fDados);

    //ordena os indices por codEstacao em ordem crscente
    if (nRegistros > 0 && listaIndices != NULL) {
        qsort(listaIndices, nRegistros, sizeof(RegistroIndice), compararIndices);
    }

    //grava os registros ordenados no arquivo de indice campo a campo
    for (int i = 0; i < nRegistros; i++) {
        fwrite(&listaIndices[i].codEstacao, sizeof(int32_t), 1, fIndice);
        fwrite(&listaIndices[i].rrn, sizeof(int32_t), 1, fIndice);
    }

    //libera memoria utilizada
    if (listaIndices) free(listaIndices);

    //muda status para '1' e reescreve o cabecalho do indice 
    cabIdx.status = '1';
    fseek(fIndice, 0, SEEK_SET);
    fwrite(&cabIdx.status, sizeof(char), 1, fIndice);

    fclose(fIndice);
    return 1;
}
