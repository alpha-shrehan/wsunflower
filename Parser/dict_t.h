#pragma once

#include "psf_gen_ast.h"
#include "psf_gen_inst_ast.h"

dict_t Sf_Dict_New(void);
dict_t *Sf_Dict_New_Ptr(void);
dict_t Sf_Dict_New_fromExpr(expr_t *, expr_t *, int);
void Sf_Dict_Push(dict_t *, expr_t, expr_t);
expr_t Sf_Dict_Pop(dict_t *, int);
expr_t Sf_Dict_Get_fromKey(dict_t *, expr_t);
void Sf_Dict_Set(dict_t *, expr_t, expr_t);