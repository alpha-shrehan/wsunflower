#include "psf_byte_t.h"

int Sf_Byte_in_Arr(psf_byte_array_t *arr, psf_byte_t val)
{
    for (size_t i = 0; i < arr->size; i++)
    {
        if (arr->nodes[i].nval_type == val.nval_type)
        {
            switch (arr->nodes[i].nval_type)
            {
            case AST_NVAL_TYPE_BOOL:
                return arr->nodes[i].v.Bool.val == val.v.Bool.val;
                break;
            case AST_NVAL_TYPE_COMMENT:
                if (!strcmp(arr->nodes[i].v.Comment.cmt, val.v.Comment.cmt))
                    return 1;
                break;
            case AST_NVAL_TYPE_DATA_TYPE:
                if (!strcmp(arr->nodes[i].v.DataType.val, val.v.DataType.val))
                    return 1;
                break;
            case AST_NVAL_TYPE_FLOAT:
                if (arr->nodes[i].v.Float.val == val.v.Float.val)
                    return 1;
                break;
            case AST_NVAL_TYPE_IDENTIFIER:
                if (!strcmp(arr->nodes[i].v.Identifier.val, val.v.Identifier.val))
                    return 1;
                break;
            case AST_NVAL_TYPE_INT:
                if (arr->nodes[i].v.Int.val == val.v.Int.val)
                    return 1;
                break;
            case AST_NVAL_TYPE_NEWLINE:
                return 1;
                break;
            case AST_NVAL_TYPE_OPERATOR:
                if (!strcmp(arr->nodes[i].v.Operator.val, val.v.Operator.val))
                    return 1;
                break;
            case AST_NVAL_TYPE_STRING:
                if (!strcmp(arr->nodes[i].v.String.val, val.v.String.val))
                    return 1;
                break;
            case AST_NVAL_TYPE_TABSPACE:
                if (arr->nodes[i].v.Tabspace.len == val.v.Tabspace.len)
                    return 1;
                break;
            default:
                break;
            }
        }
    }
    return 0;
}