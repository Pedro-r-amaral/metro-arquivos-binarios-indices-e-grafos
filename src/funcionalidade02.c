#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registros.h"
#include "io_binario.h"
#include "impressao.h"

//func2
void leitorSequencial(const char *arquivoEntrada) {
    //abrir o arquivo binário para leitura
    FILE *fp = fopen(arquivoEntrada, "rb");
    if (fp == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    //ler o status no cabeçalho
    char status;
    if (fread(&status, sizeof(char), 1, fp) != 1 || status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fp);
        return;
    }

    //pular o restante do cabeçalho
    //como o cabeçalho tem 17 bytes. ja lemos 1 byte (status), entao faltam 16
    fseek(fp, 16, SEEK_CUR);

    int registrosEncontrados = 0;
    char removido;

    //loop de leitura, tentamos ler o primeiro campo do registro ('removido')
    //se caso o fread retornar 1, significa que tem registro para ler. Se retornar 0, chegamos no EOF
    Registro reg;
    while (ler_registro_binario(fp, &reg)) {
        if (reg.removido == '1') {
            free(reg.nomeEstacao); free(reg.nomeLinha);
            continue;
        }
        imprimir_registro(&reg);
        registrosEncontrados++;
        free(reg.nomeEstacao); free(reg.nomeLinha);
    }
        //aqui realizamos o tratamento para arquivo vazio
        if (registrosEncontrados == 0) {
            printf("Registro inexistente.\n");
        }

    fclose(fp);
}