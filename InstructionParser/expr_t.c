#include "expr_t.h"

void Expr_AddToTop(expr_t *expr, int *e_sz, expr_t val)
{
    expr_t *c = OSF_Malloc(((*e_sz) + 1) * sizeof(expr_t));
    *c = val;

    for (size_t i = 1; i < (*e_sz); i++)
        c[i] = expr[i - 1];
    
    OSF_Free(expr);
    expr = OSF_Malloc(((*e_sz) + 1) * sizeof(expr_t));

    for (size_t i = 0; i < (*e_sz); i++)
        expr[i] = c[i];

    (*e_sz)++;
    OSF_Free(c);
    c = NULL;
}