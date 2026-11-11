#include "grafo.h"
#include "io_binario.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   FUNÇÕES AUXILIARES INTERNAS
   ============================================================ */

/* Compara dois strings para qsort — ordem crescente */
static int cmpStr(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* Compara dois vértices para qsort — ordem crescente por nome */
static int cmpVertice(const void *a, const void *b) {
    return strcmp(((Vertice *)a)->nomeEstacao, ((Vertice *)b)->nomeEstacao);
}

/* Busca binária no vetor de vértices pelo nome.
   Retorna o índice encontrado, ou -1 se não existir. */
int buscarVertice(Grafo *g, const char *nome) {
    int ini = 0, fim = g->numVertices - 1;
    while (ini <= fim) {
        int meio = (ini + fim) / 2;
        int cmp = strcmp(g->vertices[meio].nomeEstacao, nome);
        if (cmp == 0) return meio;
        if (cmp < 0)  ini = meio + 1;
        else          fim = meio - 1;
    }
    return -1;
}

/* Dado um array de registros e um codEstacao, retorna o nomeEstacao
   correspondente, ou NULL se não encontrado. */
static char *resolverNome(Registro *regs, int total, int cod) {
    for (int i = 0; i < total; i++) {
        if (regs[i].removido == '0' && regs[i].codEstacao == cod)
            return regs[i].nomeEstacao;
    }
    return NULL;
}

/* Insere uma linha (string) no vetor de linhas de um NoAresta,
   mantendo ordenação crescente e sem duplicatas. */
static void inserirLinha(NoAresta *no, const char *nomeLinha) {
    /* verificar duplicata */
    for (int i = 0; i < no->numLinhas; i++) {
        if (strcmp(no->linhas[i], nomeLinha) == 0) return;
    }
    /* adicionar e reordenar */
    no->linhas = realloc(no->linhas, sizeof(char *) * (no->numLinhas + 1));
    no->linhas[no->numLinhas] = strdup(nomeLinha);
    no->numLinhas++;
    qsort(no->linhas, no->numLinhas, sizeof(char *), cmpStr);
}

/* Busca ou cria um NoAresta para o destino nomeProx na lista do vértice idx.
   Se já existir, apenas insere a linha nova. Se não existir, cria o nó
   na posição correta (ordenado por nome). 
   distancia e nomeLinha são usados apenas na criação. */
static void inserirAresta(Grafo *g, int idxOrigem,
                          const char *nomeProx, int dist,
                          const char *nomeLinha) {

    Vertice *v = &g->vertices[idxOrigem];

    /* Procurar se já existe nó com esse destino */
    NoAresta *cur = v->lista;
    NoAresta *ant = NULL;
    while (cur != NULL) {
        int cmp = strcmp(cur->nomeProxEstacao, nomeProx);
        if (cmp == 0) {
            /* Destino já existe: só acrescenta a linha */
            inserirLinha(cur, nomeLinha);
            return;
        }
        if (cmp > 0) break; /* posição de inserção encontrada */
        ant = cur;
        cur = cur->prox;
    }

    /* Criar novo nó */
    NoAresta *novo = malloc(sizeof(NoAresta));
    novo->nomeProxEstacao = strdup(nomeProx);
    novo->distancia       = dist;
    novo->linhas          = malloc(sizeof(char *));
    novo->linhas[0]       = strdup(nomeLinha);
    novo->numLinhas       = 1;
    novo->prox            = cur;

    if (ant == NULL) v->lista = novo;
    else             ant->prox = novo;
}

/* ============================================================
   CONSTRUÇÃO DO GRAFO
   ============================================================ */

int construirGrafo(const char *nomeArquivo, Grafo *g) {
    FILE *fp = fopen(nomeArquivo, "rb");
    if (!fp) return 0;

    /* Ler cabeçalho */
    Cabecalho cab;
    if (fread(&cab.status,          sizeof(char), 1, fp) != 1 ||
        fread(&cab.topo,            sizeof(int),  1, fp) != 1 ||
        fread(&cab.proxRRN,         sizeof(int),  1, fp) != 1 ||
        fread(&cab.nroEstacoes,     sizeof(int),  1, fp) != 1 ||
        fread(&cab.nroParesEstacao, sizeof(int),  1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    if (cab.status != '1') { fclose(fp); return 0; }

    /* Ler todos os registros para memória */
    int capRegs = 64, totalRegs = 0;
    Registro *regs = malloc(sizeof(Registro) * capRegs);

    Registro tmp;
    while (ler_registro_binario(fp, &tmp)) {
        if (totalRegs == capRegs) {
            capRegs *= 2;
            regs = realloc(regs, sizeof(Registro) * capRegs);
        }
        regs[totalRegs++] = tmp;
    }
    fclose(fp);

    /* ---- Passo 1: coletar nomes únicos de estações (vértices) ---- */
    int capV = 64;
    g->numVertices = 0;
    g->vertices    = malloc(sizeof(Vertice) * capV);

    for (int i = 0; i < totalRegs; i++) {
        if (regs[i].removido == '1') continue;
        if (!regs[i].nomeEstacao)    continue;

        /* Verificar se o nome já foi inserido */
        int jaExiste = 0;
        for (int j = 0; j < g->numVertices; j++) {
            if (strcmp(g->vertices[j].nomeEstacao, regs[i].nomeEstacao) == 0) {
                jaExiste = 1;
                break;
            }
        }
        if (!jaExiste) {
            if (g->numVertices == capV) {
                capV *= 2;
                g->vertices = realloc(g->vertices, sizeof(Vertice) * capV);
            }
            g->vertices[g->numVertices].nomeEstacao = strdup(regs[i].nomeEstacao);
            g->vertices[g->numVertices].lista        = NULL;
            g->numVertices++;
        }
    }

    /* Ordenar vetor de vértices por nome */
    qsort(g->vertices, g->numVertices, sizeof(Vertice), cmpVertice);

    /* ---- Passo 2: inserir arestas ---- */
    for (int i = 0; i < totalRegs; i++) {
        if (regs[i].removido == '1') continue;
        if (!regs[i].nomeEstacao)    continue;

        int idxOrigem = buscarVertice(g, regs[i].nomeEstacao);
        if (idxOrigem < 0) continue; /* não deveria acontecer */

        /* Aresta tipo 1: estacao → proxEstacao */
        if (regs[i].codProxEstacao != VALOR_NULO_INT) {
            char *nomeProx = resolverNome(regs, totalRegs, regs[i].codProxEstacao);
            if (nomeProx && regs[i].nomeLinha) {
                inserirAresta(g, idxOrigem, nomeProx,
                              regs[i].distProxEstacao, regs[i].nomeLinha);
            }
        }

        /* Aresta tipo 2: estacao → estacaoIntegracao (dist=0, linha="Integração") */
        if (regs[i].codEstIntegra != VALOR_NULO_INT) {
            char *nomeIntegra = resolverNome(regs, totalRegs, regs[i].codEstIntegra);
            if (nomeIntegra &&
                strcmp(regs[i].nomeEstacao, nomeIntegra) != 0) {
                inserirAresta(g, idxOrigem, nomeIntegra, 0, "Integração");
            }
        }
    }

    /* Liberar array de registros temporários */
    for (int i = 0; i < totalRegs; i++) {
        free(regs[i].nomeEstacao);
        free(regs[i].nomeLinha);
    }
    free(regs);

    return 1;
}

/* ============================================================
   IMPRESSÃO — formato da funcionalidade [10]
   Exemplo: Luz Bras 2310 Coral Jade Rubi Republica 1257 Amarela ...
   (tudo numa linha por vértice, separado por espaços)
   ============================================================ */
void imprimirGrafo(Grafo *g) {
    for (int i = 0; i < g->numVertices; i++) {
        /* Não imprimir vértices sem arestas */
        if (g->vertices[i].lista == NULL) continue;

        printf("%s", g->vertices[i].nomeEstacao);

        NoAresta *cur = g->vertices[i].lista;
        while (cur != NULL) {
            printf(", %s, %d", cur->nomeProxEstacao, cur->distancia);
            for (int k = 0; k < cur->numLinhas; k++) {
                printf(", %s", cur->linhas[k]);
            }
            cur = cur->prox;
        }
        printf("\n");
    }
}

/* ============================================================
   LIBERAÇÃO DE MEMÓRIA
   ============================================================ */
void liberarGrafo(Grafo *g) {
    for (int i = 0; i < g->numVertices; i++) {
        free(g->vertices[i].nomeEstacao);
        NoAresta *cur = g->vertices[i].lista;
        while (cur) {
            NoAresta *prox = cur->prox;
            free(cur->nomeProxEstacao);
            for (int k = 0; k < cur->numLinhas; k++) free(cur->linhas[k]);
            free(cur->linhas);
            free(cur);
            cur = prox;
        }
    }
    free(g->vertices);
    g->vertices    = NULL;
    g->numVertices = 0;
}