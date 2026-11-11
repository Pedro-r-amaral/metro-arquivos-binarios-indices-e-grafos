#ifndef REGISTROS_H
#define REGISTROS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//definição de valores nulos e lixo
#define VALOR_NULO_INT -1
#define CARACTERE_LIXO '$'
#define TAM_REGISTRO_DADOS 80
#define TAM_CABECALHO 17


//struct do cabeçalho
typedef struct {
    char status;        // '0' ou '1'
    int topo;           // RRN do primeiro removido ou -1
    int proxRRN;        // próximo RRN disponível
    int nroEstacoes;    // quantidade de estações únicas
    int nroParesEstacao;// quantidade de pares únicos
} Cabecalho;



//struct de dados
typedef struct {
    //controle
    char removido;     //'0' ou '1'
    int proximo;       //RRN do próximo removido
    
    //campos de dados fixos
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    
    //campos de dados variáveis
    int tamNomeEstacao;
    char *nomeEstacao;
    
    int tamNomeLinha;
    char *nomeLinha;
} Registro;

//tamanho fixo das estruturas do indice em disco
#define TAM_CABECALHO_INDICE 1
#define TAM_REGISTRO_INDICE 8

//struct cabecalho do indice primario 
typedef struct {
    char status; //'0' para inconsistente, '1' para consistente
} CabecalhoIndice;

//struct de dados do indice primario
typedef struct {
    int32_t codEstacao; //chave primaria
    int32_t rrn;        //rrn correspondente no arquivo de dados 
} RegistroIndice;



//prototipos funcionalidades 1, 2, 3 e 4 (parte entrega 1)
int CSVparaBin(char *nomeArquivoCSV, char *nomeArquivoBin);
void leitorSequencial(const char *arquivoEntrada);
void criterioBusca(char *nomeArquivoBin);
void acessoDiretoRRN(const char *arquivoEntrada, int rrn);

//prototipos funcionalidades 5, 6, 7, 8 e 9 (parte entrega 2)
int criarIndicePrimario(const char *nomeArquivoDados, const char *nomeArquivoIndice);
void buscaIndexada(char *nomeArquivoDados, char *nomeArquivoIndice);
void remocaoLogica(char *nomeArquivoDados, char *nomeArquivoIndice);
void insercaoDados(char *nomeArquivoDados, char *nomeArquivoIndice);
void atualizacaoDados(char *nomeArquivoDados, char *nomeArquivoIndice);

//prototipos funcionalidades 10, 11, 12 e 13 (entrega 3)
void funcionalidade10(void);
void funcionalidade11(void);
void funcionalidade12(void);
void funcionalidade13(void);
#endif