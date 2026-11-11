// impressao.c — tirado de funcionalidade03.c, sem alteração
#include "impressao.h"
#include <stdio.h>

//exibir dados formatados
void imprimir_registro(Registro *r) {
    printf("%d ", r->codEstacao);
    printf("%s ", (r->tamNomeEstacao > 0) ? r->nomeEstacao : "NULO");
    
    if (r->codLinha != VALOR_NULO_INT) printf("%d ", r->codLinha);
    else printf("NULO ");
    
    printf("%s ", (r->tamNomeLinha > 0) ? r->nomeLinha : "NULO");
    
    if (r->codProxEstacao != VALOR_NULO_INT) printf("%d ", r->codProxEstacao);
    else printf("NULO ");
    
    if (r->distProxEstacao != VALOR_NULO_INT) printf("%d ", r->distProxEstacao);
    else printf("NULO ");
    
    if (r->codLinhaIntegra != VALOR_NULO_INT) printf("%d ", r->codLinhaIntegra);
    else printf("NULO ");
    
    if (r->codEstIntegra != VALOR_NULO_INT) printf("%d\n", r->codEstIntegra);
    else printf("NULO\n");
}
