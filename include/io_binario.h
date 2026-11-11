// io_binario.h — declara funções de leitura/escrita no arquivo binário
#ifndef IO_BINARIO_H
#define IO_BINARIO_H

#include <stdio.h>
#include "registros.h"

void escreverCabecalho(FILE *fp, Cabecalho *c);
void escreverRegistro(FILE *fp, Registro *r);
int  ler_registro_binario(FILE *fp, Registro *r);

#endif