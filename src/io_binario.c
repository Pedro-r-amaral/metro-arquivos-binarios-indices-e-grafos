// io_binario.c — implementação das funções de I/O binário
#include "io_binario.h"
#include <stdlib.h>
#include <string.h>

// Tirada de funcionalidade01
void escreverCabecalho(FILE *fp, Cabecalho *c) {
    fseek(fp, 0, SEEK_SET);
    fwrite(&c->status,          sizeof(char), 1, fp);
    fwrite(&c->topo,            sizeof(int),  1, fp);
    fwrite(&c->proxRRN,         sizeof(int),  1, fp);
    fwrite(&c->nroEstacoes,     sizeof(int),  1, fp);
    fwrite(&c->nroParesEstacao, sizeof(int),  1, fp);
}

// Tirada de funcionalidade01
void escreverRegistro(FILE *fp, Registro *r) {
    fwrite(&r->removido, sizeof(char), 1, fp);
    fwrite(&r->proximo, sizeof(int), 1, fp);
    fwrite(&r->codEstacao, sizeof(int), 1, fp);
    fwrite(&r->codLinha, sizeof(int), 1, fp);
    fwrite(&r->codProxEstacao, sizeof(int), 1, fp);
    fwrite(&r->distProxEstacao, sizeof(int), 1, fp);
    fwrite(&r->codLinhaIntegra, sizeof(int), 1, fp);
    fwrite(&r->codEstIntegra, sizeof(int), 1, fp);
    
    //variáveis: indicador de tamanho + string(sem \0)
    fwrite(&r->tamNomeEstacao, sizeof(int), 1, fp);
    if (r->tamNomeEstacao > 0)
        fwrite(r->nomeEstacao, sizeof(char), r->tamNomeEstacao, fp);
        
    fwrite(&r->tamNomeLinha, sizeof(int), 1, fp);
    if (r->tamNomeLinha > 0)
        fwrite(r->nomeLinha, sizeof(char), r->tamNomeLinha, fp);
        
    //colocando $ até 80 bytes
    int bytesEscritos = 37 + r->tamNomeEstacao + r->tamNomeLinha;
    //37 = removido(1) + 7 campos int(28) + tamNomeEstacao(4) + tamNomeLinha(4)
    char lixo = CARACTERE_LIXO;
    for (int i = bytesEscritos; i < TAM_REGISTRO_DADOS; i++) {
        fwrite(&lixo, sizeof(char), 1, fp);
    }
}

// Tirada de funcionalidade03
//funcao para ler registro do arquivo binario
int ler_registro_binario(FILE *fp, Registro *r) {
    if (fread(&r->removido, sizeof(char), 1, fp) != 1) return 0;

    // Se estiver removido, pula o restante do registro imediatamente e retorna sucesso na leitura do esqueleto
    if (r->removido == '1') {
        fseek(fp, TAM_REGISTRO_DADOS - 1, SEEK_CUR);
        r->nomeEstacao = NULL;
        r->nomeLinha = NULL;
        return 1;
    }

    //campos fixos
    fread(&r->proximo, sizeof(int32_t), 1, fp);
    fread(&r->codEstacao, sizeof(int32_t), 1, fp);
    fread(&r->codLinha, sizeof(int32_t), 1, fp);
    fread(&r->codProxEstacao, sizeof(int32_t), 1, fp);
    fread(&r->distProxEstacao, sizeof(int32_t), 1, fp);
    fread(&r->codLinhaIntegra, sizeof(int32_t), 1, fp);
    fread(&r->codEstIntegra, sizeof(int32_t), 1, fp);

    //campos variaveis
    fread(&r->tamNomeEstacao, sizeof(int32_t), 1, fp);
    if (r->tamNomeEstacao > 0) {
        r->nomeEstacao = (char *) malloc(r->tamNomeEstacao + 1);
        fread(r->nomeEstacao, sizeof(char), r->tamNomeEstacao, fp);
        r->nomeEstacao[r->tamNomeEstacao] = '\0';
    } else {
        r->nomeEstacao = NULL;
    }

    //campos variaveis
    fread(&r->tamNomeLinha, sizeof(int32_t), 1, fp);
    if (r->tamNomeLinha > 0) {
        r->nomeLinha = (char *) malloc(r->tamNomeLinha + 1);
        fread(r->nomeLinha, sizeof(char), r->tamNomeLinha, fp);
        r->nomeLinha[r->tamNomeLinha] = '\0';
    } else {
        r->nomeLinha = NULL;
    }

    //pula os lixos $
    int bytesLidos = 1 + 4 + (6 * 4) + 4 + r->tamNomeEstacao + 4 + r->tamNomeLinha;
    fseek(fp, TAM_REGISTRO_DADOS - bytesLidos, SEEK_CUR);
    
    return 1;
}