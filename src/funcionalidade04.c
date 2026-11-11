#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"
#include "impressao.h"

void acessoDiretoRRN(const char *arquivoEntrada, int rrn) {
    FILE *fp = fopen(arquivoEntrada, "rb");
    if (fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    char status;
    if (fread(&status, sizeof(char), 1, fp) != 1 || status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp);
        return;
    }

    long byteOffset = TAM_CABECALHO + (long)rrn * TAM_REGISTRO_DADOS;
    fseek(fp, byteOffset, SEEK_SET);

    Registro reg;
    if (!ler_registro_binario(fp, &reg)) {
        printf("Registro inexistente.\n");
        fclose(fp);
        return;
    }

    fclose(fp);

    if (reg.removido == '1') {
        printf("Registro inexistente.\n");
        free(reg.nomeEstacao);
        free(reg.nomeLinha);
        return;
    }

    imprimir_registro(&reg);
    free(reg.nomeEstacao);
    free(reg.nomeLinha);
}