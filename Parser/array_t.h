#pragma once

#include "psf_gen_ast.h"
#include "psf_gen_inst_ast.h"

array_t Sf_Array_New(void);
array_t *Sf_Array_New_Ptr(void);
array_t Sf_Array_New_fromExpr(expr_t *, int);
void Sf_Array_Push(array_t *, expr_t);
expr_t Sf_Array_Pop(array_t *, int);