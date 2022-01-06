#include "array_t.h"

array_t Sf_Array_New(void)
{
    array_t arr;
    arr.len = 0;
    arr.vals = OSF_Malloc(sizeof(expr_t));
    arr.parent = NULL;

    return arr;
}

array_t *Sf_Array_New_Ptr(void)
{
    array_t *arr = OSF_Malloc(sizeof(expr_t));
    *arr = Sf_Array_New();

    return arr;
}

array_t Sf_Array_New_fromExpr(expr_t *ea, int sz)
{
    array_t n = Sf_Array_New();
    n.vals = ea;
    n.len = sz;
    n.parent = NULL;

    return n;
}

void Sf_Array_Push(array_t *arr, expr_t val)
{
    if (arr->len)
        arr->vals = OSF_Realloc(arr->vals, (arr->len + 1) * sizeof(expr_t));
    
    arr->vals[arr->len++] = val;

    if (arr->parent != NULL)
        Sf_Array_Push(arr->parent, val);
}
