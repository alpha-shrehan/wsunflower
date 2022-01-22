#include "dict_t.h"
#include "array_t.h"

dict_t Sf_Dict_New(void)
{
    dict_t nd;
    nd.evaluated = 0;
    nd.keys = OSF_Malloc(sizeof(expr_t));
    nd.vals = OSF_Malloc(sizeof(expr_t));
    nd.len = 0;

    return nd;
}

dict_t *Sf_Dict_New_Ptr(void)
{
    dict_t *nd = OSF_Malloc(sizeof(dict_t));
    *nd = Sf_Dict_New();

    return nd;
}

dict_t Sf_Dict_New_fromExpr(expr_t *keys, expr_t *vals, int sz)
{
    dict_t nd = Sf_Dict_New();
    nd.len = sz;

    for (size_t i = 0; i < sz; i++)
    {
        if (i)
        {
            nd.keys = OSF_Realloc(nd.keys, (i + 1) * sizeof(expr_t));
            nd.vals = OSF_Realloc(nd.vals, (i + 1) * sizeof(expr_t));
        }

        nd.keys[i] = keys[i];
        nd.vals[i] = vals[i];
    }

    return nd;
}

void Sf_Dict_Push(dict_t *nd, expr_t k, expr_t v)
{
    if (nd->len)
    {
        nd->keys = OSF_Realloc(nd->keys, (nd->len + 1) * sizeof(expr_t));
        nd->vals = OSF_Realloc(nd->vals, (nd->len + 1) * sizeof(expr_t));
    }

    nd->keys[nd->len] = k;
    nd->vals[nd->len] = v;
    nd->len++;
}

expr_t Sf_Dict_Pop(dict_t *nd, int idx)
{
    if (idx < 0)
        idx += nd->len;

    expr_t k = nd->keys[idx], v = nd->vals[idx];

    expr_t *copy_k = OSF_Malloc((nd->len - 1) * sizeof(expr_t)),
           *copy_v = OSF_Malloc((nd->len - 1) * sizeof(expr_t));
    int pivot = 0;

    for (size_t i = 0; i < nd->len; i++)
    {
        if (i == idx)
            continue;
        copy_k[pivot++] = nd->keys[i];
    }
    pivot = 0;

    for (size_t i = 0; i < nd->len; i++)
    {
        if (i == idx)
            continue;
        copy_v[pivot++] = nd->vals[i];
    }

    OSF_Free(nd->keys);
    OSF_Free(nd->vals);

    nd->keys = copy_k;
    nd->vals = copy_v;
    nd->len--;

    expr_t res;
    res.type = EXPR_TYPE_CONSTANT;
    res.v.constant.constant_type = CONSTANT_TYPE_ARRAY;
    array_t av = Sf_Array_New();
    av.evaluated = 1;
    av.len = 2;
    av.vals = OSF_Realloc(av.vals, 2 * sizeof(expr_t));

    av.vals[0] = k;
    av.vals[1] = v;

    res.v.constant.Array.index = PSG_AddArray(av);

    return res;
}

expr_t Sf_Dict_Get_fromKey(dict_t *nd, expr_t key)
{
    for (size_t i = 0; i < nd->len; i++)
    {
        if (_IPSF_ExecLogicalArithmetic(nd->keys[i], LOGICAL_OP_EQEQ, key).v.constant.Bool.value)
            return nd->vals[i];
    }

    assert(0 && SF_FMT("Error: Dict has no member %s."));
}