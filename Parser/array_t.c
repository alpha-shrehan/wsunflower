#include "array_t.h"

array_t Sf_Array_New(void)
{
    array_t arr;
    arr.len = 0;
    arr.vals = OSF_Malloc(sizeof(expr_t));
    // arr.parent = NULL;
    arr.evaluated = 0;

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
    // n.parent = NULL;
    n.evaluated = 0;

    return n;
}

void Sf_Array_Push(array_t *arr, expr_t val)
{
    if (arr->len)
        arr->vals = OSF_Realloc(arr->vals, (arr->len + 1) * sizeof(expr_t));
    
    arr->vals[arr->len++] = val;
}

expr_t Sf_Array_Pop(array_t *arr, int index)
{    
    if (index < 0)
        index += arr->len;
    
    assert(index < arr->len && SF_FMT("Error: Index out of range."));
    
    array_t *ar = Sf_Array_New_Ptr();
    expr_t pres = arr->vals[index];

    for (size_t i = 0; i < arr->len; i++)
    {
        if (i == index)
            continue;
        Sf_Array_Push(ar, arr->vals[i]);
    }

    OSF_Free(arr->vals);
    arr->vals = ar->vals;
    arr->len = ar->len;

    return pres;
}