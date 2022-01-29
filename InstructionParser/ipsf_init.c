#include "ipsf_init.h"
#include <Parser/psf_byte_t.h>
#include <Parser/array_t.h>
#include <Parser/dict_t.h>
#include <builtins/built_in.h>

#ifndef WIN32
char *strrev(char *str)
{
    char *p1, *p2;

    if (!str || !*str)
        return str;

    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }

    return str;
}
#endif

/**
 * MAIN:
 */
expr_t *IPSF_ExecIT_fromMod(mod_t *mod, int *err)
{
    expr_t *RET = (expr_t *)OSF_Malloc(sizeof(expr_t));
    *RET = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_DTYPE,
                .DType = {
                    .type = DATA_TYPE_NONE}}},
        .next = NULL,
        .line = BODY(mod)->body->line};

    if (err != NULL)
        *err = IPSF_OK;

    for (size_t i = 0; i < BODY(mod)->body_size; i++)
    {
        /**
         * @brief backtrace should be empty here
         */
        if (*OSF_GetBacklogSize())
            OSF_ClearBacklog();

        stmt_t curr = BODY(mod)->body[i];
        OSF_AddBackLog(OSF_CreateBackLog(BACKTRACE_ENTITY_STMT, curr.line));

        int doBreak = 0;

        switch (curr.type)
        {
        case STATEMENT_TYPE_VAR_DECL:
        {
            struct __mod_child_varhold_s *expr = IPSF_ExecVarDecl_fromStmt(mod, curr, err);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            if (expr != NULL)
                *RET = expr->val;
        }
        break;
        case STATEMENT_TYPE_VAR_REF:
        {
            struct __mod_child_varhold_s *expr = IPSF_GetVar_fromMod(mod, curr.v.var_ref.name, err);

            if (expr == NULL)
            {
                OSF_SetException((except_t){
                    .type = EXCEPT_VARIABLE_DOES_NOT_EXIST,
                    .line = curr.line,
                    .v.ce1 = {
                        .m_ref = mod,
                        .vname = curr.v.var_ref.name}});

                goto __label_abrupt_end_IPSF_ExecIT_fromMod;
            }

            *RET = expr->val;
        }
        break;
        case STATEMENT_TYPE_EXPR:
        {
            expr_t *ex_res = IPSF_ExecExprStatement_fromMod(mod, curr, err);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            expr_t _fex = *ex_res;
            OSF_Free(ex_res);
            *RET = _fex;
        }
        break;
        case STATEMENT_TYPE_IF:
        {
            expr_t cond_e = IPSF_ReduceExpr_toConstant(mod, *(curr.v.if_stmt.condition));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            if (_PSF_EntityIsTrue(cond_e))
            {
                mod_t *body_mod = SF_CreateModule(mod->type, NULL);
                body_mod->parent = mod;
                OSF_Free(BODY(body_mod)->body);
                BODY(body_mod)->body = curr.v.if_stmt.body;
                BODY(body_mod)->body_size = curr.v.if_stmt.body_size;
                OSF_Free(IPSF_ExecIT_fromMod(body_mod, err));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                if (body_mod->returns != NULL)
                {
                    mod->returns = body_mod->returns;
                    doBreak = 1;
                }

                SF_Module_safeDelete(body_mod);
                OSF_Free(body_mod);

                if (doBreak)
                    break;
            }
            else
            {
                int condition_satisfied = 0;

                // Evaluate elif conditions
                for (size_t j = 0; j < curr.v.if_stmt.elif_stmts_count; j++)
                {
                    struct _conditional_struct c = curr.v.if_stmt.elif_stmts[j];
                    if (c.condition == NULL)
                        exit(printf("Error: seg_fault\n"));
                    expr_t _ce = IPSF_ReduceExpr_toConstant(mod, *(c.condition));

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                    if (_PSF_EntityIsTrue(_ce))
                    {
                        mod_t *body_mod = SF_CreateModule(mod->type, NULL);
                        body_mod->parent = mod;
                        OSF_Free(BODY(body_mod)->body);
                        BODY(body_mod)->body = c.body;
                        BODY(body_mod)->body_size = c.body_size;
                        OSF_Free(IPSF_ExecIT_fromMod(body_mod, err));

                        if (OSF_GetExceptionState())
                            goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                        if (body_mod->returns != NULL)
                        {
                            mod->returns = body_mod->returns;
                            doBreak = 1;
                        }

                        SF_Module_safeDelete(body_mod);
                        OSF_Free(body_mod);

                        condition_satisfied = 1;

                        if (doBreak)
                            break;
                    }
                }

                if (doBreak)
                    break;

                if (!condition_satisfied)
                {
                    if (curr.v.if_stmt.else_stmt != NULL)
                    {
                        mod_t *body_mod = SF_CreateModule(mod->type, NULL);
                        body_mod->parent = mod;
                        OSF_Free(BODY(body_mod)->body);
                        BODY(body_mod)->body = curr.v.if_stmt.else_stmt->body;
                        BODY(body_mod)->body_size = curr.v.if_stmt.else_stmt->body_size;
                        OSF_Free(IPSF_ExecIT_fromMod(body_mod, err));

                        if (OSF_GetExceptionState())
                            goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                        if (body_mod->returns != NULL)
                        {
                            mod->returns = body_mod->returns;
                            doBreak = 1;
                        }

                        SF_Module_safeDelete(body_mod);
                        OSF_Free(body_mod);

                        if (doBreak)
                            break;
                    }
                }
            }
        }
        break;
        case STATEMENT_TYPE_WHILE:
        {
            expr_t cond_r = IPSF_ReduceExpr_toConstant(mod, *(curr.v.while_stmt.condition));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            mod_t *w_mod = SF_CreateModule(mod->type, NULL);
            w_mod->parent = mod;
            OSF_Free(BODY(w_mod)->body);

            while (_PSF_EntityIsTrue(cond_r))
            {
                BODY(w_mod)->body = curr.v.while_stmt.body;
                BODY(w_mod)->body_size = curr.v.while_stmt.body_size;
                // printf("[x]"); OSF_PrintHeapSize(); printf("\n");
                OSF_Free(IPSF_ExecIT_fromMod(w_mod, err));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                cond_r = IPSF_ReduceExpr_toConstant(mod, *(curr.v.while_stmt.condition));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                if (w_mod->returns != NULL)
                {
                    mod->returns = w_mod->returns;
                    doBreak = 1;
                }

                SF_Module_safeDelete(w_mod);
                w_mod->var_holds_size = 0;
                w_mod->var_holds = OSF_Malloc(sizeof(var_t));

                if (doBreak)
                    break;
                // printf("[y]"); OSF_PrintHeapSize(); printf("\n");
            }

            SF_Module_safeDelete(w_mod);
            OSF_Free(w_mod);
        }
        break;
        case STATEMENT_TYPE_FOR:
        {
            expr_t cond_red = IPSF_ReduceExpr_toConstant(mod, *(curr.v.for_stmt.condition));
            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            int pres_body_type = mod->type;
            int pres_body_size = BODY(mod)->body_size;
            stmt_t *pres_body = BODY(mod)->body;
            char *pres_name = BODY(mod)->name;
            var_t *pres_vars = mod->var_holds;
            int pres_var_size = mod->var_holds_size;

            mod_t *f_mod = /* SF_CreateModule(mod->type, NULL) */ mod;
            mod_t *f_cache_mod = SF_CreateModule(mod->type, NULL);
            // f_mod->parent = mod;
            // OSF_Free(BODY(f_mod)->body);

            switch (cond_red.v.constant.constant_type)
            {
            case CONSTANT_TYPE_ARRAY:
            {
                for (size_t j = 0; j < ARRAY(cond_red.v.constant.Array.index).len; j++)
                {
                    expr_t idx_val = ARRAY(cond_red.v.constant.Array.index).vals[j];

                    switch (idx_val.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_ARRAY:
                    {
                        expr_t *arr_ref = ARRAY(idx_val.v.constant.Array.index).vals;
                        int sz_ref = ARRAY(idx_val.v.constant.Array.index).len;

                        if (curr.v.for_stmt.var_size == sz_ref)
                        {
                            for (size_t k = 0; k < sz_ref; k++)
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);

                                _IPSF_AddVar_toModule(
                                    f_cache_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);
                            }
                        }
                        else if (curr.v.for_stmt.var_size < sz_ref)
                        {
                            for (size_t k = 0; k < curr.v.for_stmt.var_size; k++)
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);

                                _IPSF_AddVar_toModule(
                                    f_cache_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);
                            }
                        }
                        else if (curr.v.for_stmt.var_size > sz_ref)
                        {
                            for (size_t k = 0; k < sz_ref; k++)
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);

                                _IPSF_AddVar_toModule(
                                    f_cache_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);
                            }
                        }
                    }
                    break;

                    default:
                    {
                        _IPSF_AddVar_toModule(
                            f_mod,
                            curr.v.for_stmt.vars[0].name,
                            idx_val);

                        _IPSF_AddVar_toModule(
                            f_cache_mod,
                            curr.v.for_stmt.vars[0].name,
                            idx_val);
                    }
                    break;
                    }

                    // Add variables as a check afterhand
                    for (size_t k = 0; k < curr.v.for_stmt.var_size; k++)
                    {
                        int err_c = IPSF_OK;
                        var_t *pv_v = IPSF_GetVar_fromMod(f_cache_mod, curr.v.for_stmt.vars[k].name, &err_c);

                        /**
                         * @brief Variables who could not fit to data
                         * will not be declared in the module.
                         * But ignoring them as symbols is unethical if such symbols
                         * have a default value.
                         * So, they will be added here ONLY IF they have a default value
                         */
                        if (err_c == IPSF_ERR_VAR_NOT_FOUND)
                        {
                            expr_t _vred = IPSF_ReduceExpr_toConstant(f_mod, curr.v.for_stmt.vars[k].val);

                            if (OSF_GetExceptionState())
                                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                            if (!_IPSF_IsDataType_Void(_vred))
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    _vred);

                                _IPSF_AddVar_toModule(
                                    f_cache_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    _vred);
                            }

                            continue;
                        }
                    }

                    BODY(f_mod)->body = curr.v.for_stmt.body;
                    BODY(f_mod)->body_size = curr.v.for_stmt.body_size;

                    OSF_Free(IPSF_ExecIT_fromMod(f_mod, err));

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                    if (f_mod->returns != NULL)
                    {
                        mod->returns = f_mod->returns;
                        doBreak = 1;
                    }

                    SF_Module_safeDelete(f_cache_mod);
                    f_cache_mod->var_holds = OSF_Malloc(sizeof(var_t));
                    f_cache_mod->var_holds_size = 0;

                    // SF_Module_safeDelete(f_mod);
                    // f_mod->var_holds = OSF_Malloc(sizeof(var_t));
                    // f_mod->var_holds_size = 0;

                    if (doBreak)
                        break;
                }
            }
            break;
            case CONSTANT_TYPE_CLASS_OBJECT:
            {
                class_t *_c_ref = _IPSF_GetClass_fromIntTuple(cond_red.v.constant.ClassObj.idx);
                var_t *iter_conf = IPSF_GetVar_fromMod(_c_ref->mod, "operatorfor..in", NULL);
                assert(iter_conf != NULL && SF_FMT("Error: Class object is not an iterable."));
                assert(iter_conf->val.type == EXPR_TYPE_FUNCTION && SF_FMT("Error: `operator for..in' is not a function."));
                fun_t _get_iter = (*PSG_GetFunctions())[iter_conf->val.v.function_s.index];

                expr_t *_itc_val = _IPSF_CallFunction(_get_iter, (expr_t *)(expr_t[]){cond_red}, 1, _c_ref->mod->parent);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                while (!_IPSF_IsDataType_None(*_itc_val))
                {
                    expr_t idx_val = *_itc_val;
                    assert((
                               idx_val.type == EXPR_TYPE_CONSTANT &&
                               idx_val.v.constant.constant_type == CONSTANT_TYPE_CLASS_OBJECT) &&
                           SF_FMT("Error: Iterative must be a class object."));

                    class_t *c_get = _IPSF_GetClass_fromIntTuple(idx_val.v.constant.ClassObj.idx);
                    var_t *get_iter_repr = IPSF_GetVar_fromMod(c_get->mod, "__iter_repr__", NULL);
                    assert(get_iter_repr != NULL && SF_FMT("Error: Class object is not an iterative."));
                    assert(get_iter_repr->val.type == EXPR_TYPE_FUNCTION && SF_FMT("Error: `__iter_repr__' is not a function."));
                    fun_t _get_iter_repr = (*PSG_GetFunctions())[get_iter_repr->val.v.function_s.index];

                    expr_t *idx_v_dy = _IPSF_CallFunction(_get_iter_repr, (expr_t *)(expr_t[]){idx_val}, 1, c_get->mod->parent);

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                    idx_val = *idx_v_dy;
                    OSF_Free(idx_v_dy);

                    switch (idx_val.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_ARRAY:
                    {
                        expr_t *arr_ref = ARRAY(idx_val.v.constant.Array.index).vals;
                        int sz_ref = ARRAY(idx_val.v.constant.Array.index).len;

                        if (curr.v.for_stmt.var_size == sz_ref)
                        {
                            for (size_t k = 0; k < sz_ref; k++)
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);
                            }
                        }
                        else if (curr.v.for_stmt.var_size < sz_ref)
                        {
                            for (size_t k = 0; k < curr.v.for_stmt.var_size; k++)
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);
                            }
                        }
                        else if (curr.v.for_stmt.var_size > sz_ref)
                        {
                            for (size_t k = 0; k < sz_ref; k++)
                            {
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    arr_ref[k]);
                            }
                        }
                    }
                    break;

                    default:
                    {
                        _IPSF_AddVar_toModule(
                            f_mod,
                            curr.v.for_stmt.vars[0].name,
                            idx_val);
                    }
                    break;
                    }

                    // Add variables as a check afterhand
                    for (size_t k = 0; k < curr.v.for_stmt.var_size; k++)
                    {
                        int err_c = IPSF_OK;
                        var_t *pv_v = IPSF_GetVar_fromMod(f_mod, curr.v.for_stmt.vars[k].name, &err_c);

                        /**
                         * @brief Variables who could not fit to data
                         * will not be declared in the module.
                         * But ignoring them as symbols is unethical if such symbols
                         * have a default value.
                         * So, they will be added here ONLY IF they have a default value
                         */
                        if (err_c == IPSF_ERR_VAR_NOT_FOUND)
                        {
                            expr_t _vred = IPSF_ReduceExpr_toConstant(f_mod, curr.v.for_stmt.vars[k].val);

                            if (OSF_GetExceptionState())
                                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                            if (!_IPSF_IsDataType_Void(_vred))
                                _IPSF_AddVar_toModule(
                                    f_mod,
                                    curr.v.for_stmt.vars[k].name,
                                    _vred);

                            continue;
                        }
                    }

                    BODY(f_mod)->body = curr.v.for_stmt.body;
                    BODY(f_mod)->body_size = curr.v.for_stmt.body_size;

                    OSF_Free(IPSF_ExecIT_fromMod(f_mod, err));

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                    if (f_mod->returns != NULL)
                    {
                        mod->returns = f_mod->returns;
                        doBreak = 1;
                    }

                    // SF_Module_safeDelete(f_mod);
                    // f_mod->var_holds = OSF_Malloc(sizeof(var_t));
                    // f_mod->var_holds_size = 0;

                    if (doBreak)
                        break;

                    OSF_Free(_itc_val);
                    _itc_val = _IPSF_CallFunction(_get_iter, (expr_t *)(expr_t[]){cond_red}, 1, _c_ref->mod->parent);
                }

                OSF_Free(_itc_val);
            }
            break;
            case CONSTANT_TYPE_DICT:
            {
                dict_t *dc = PSG_GetDict_Ptr(cond_red.v.constant.Dict.index);

                if (!dc->evaluated)
                    IPSF_ReduceExpr_toConstant(mod, cond_red);

                assert(
                    curr.v.for_stmt.var_size <= 2 &&
                    SF_FMT("Error: Dictionary iteration allows a maximum of 2 substitution variables."));

                for (size_t j = 0; j < dc->len; j++)
                {
                    expr_t key = dc->keys[j],
                           val = dc->vals[j];

                    if (curr.v.for_stmt.var_size == 1)
                    {
                        expr_t *kvv = OSF_Malloc(2 * sizeof(expr_t));
                        kvv[0] = key;
                        kvv[1] = val;

                        array_t ac = Sf_Array_New_fromExpr(kvv, 2);

                        expr_t v = (expr_t){
                            .type = EXPR_TYPE_CONSTANT,
                            .v = {
                                .constant = {
                                    .constant_type = CONSTANT_TYPE_ARRAY,
                                    .Array = {
                                        .index = PSG_AddArray(ac)}}},
                            .line = curr.line};

                        _IPSF_AddVar_toModule(
                            f_mod,
                            (*(curr.v.for_stmt.vars)).name,
                            v);
                    }
                    else
                    {
                        _IPSF_AddVar_toModule(
                            f_mod,
                            (*(curr.v.for_stmt.vars)).name,
                            key);

                        _IPSF_AddVar_toModule(
                            f_mod,
                            curr.v.for_stmt.vars[1].name,
                            val);
                    }

                    BODY(f_mod)->body = curr.v.for_stmt.body;
                    BODY(f_mod)->body_size = curr.v.for_stmt.body_size;

                    OSF_Free(IPSF_ExecIT_fromMod(f_mod, err));

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                    if (f_mod->returns != NULL)
                    {
                        mod->returns = f_mod->returns;
                        doBreak = 1;
                    }

                    // SF_Module_safeDelete(f_mod);
                    // f_mod->var_holds = OSF_Malloc(sizeof(var_t));
                    // f_mod->var_holds_size = 0;

                    if (doBreak)
                        break;
                }
            }
            break;
            default:
                break;
            }

            mod->type = pres_body_type;
            BODY(mod)->body = pres_body;
            BODY(mod)->body_size = pres_body_size;
            BODY(mod)->name = pres_name;

            SF_Module_safeDelete(f_cache_mod);
            OSF_Free(f_cache_mod);
        }
        break;
        case STATEMENT_TYPE_REPEAT:
        {
            expr_t cond_tok = IPSF_ReduceExpr_toConstant(mod, *(curr.v.repeat_stmt.condition));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            assert((cond_tok.type == EXPR_TYPE_CONSTANT &&
                    cond_tok.v.constant.constant_type == CONSTANT_TYPE_INT) &&
                   SF_FMT("Error: Iteration count must be an integer."));

            int cond_iters = cond_tok.v.constant.Int.value;

            mod_t *body_mod = SF_CreateModule(mod->type, NULL);
            body_mod->parent = mod;
            OSF_Free(BODY(body_mod)->body);

            while (cond_iters--)
            {
                BODY(body_mod)->body = curr.v.repeat_stmt.body;
                BODY(body_mod)->body_size = curr.v.repeat_stmt.body_size;

                OSF_Free(IPSF_ExecIT_fromMod(body_mod, NULL));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                if (body_mod->returns != NULL)
                {
                    mod->returns = body_mod->returns;
                    doBreak = 1;
                    break;
                }
            }

            SF_Module_safeDelete(body_mod);
            OSF_Free(body_mod);
        }
        break;
        case STATEMENT_TYPE_FUNCTION_DECL:
        {
            const char *fname = (const char *)curr.v.function_decl.name;

            fun_t _fdef;
            _fdef.name = (char *)fname;
            _fdef.is_native = 0;
            _fdef.arg_acceptance_count = curr.v.function_decl.arg_size;
            _fdef.v.Coded.arg_size = curr.v.function_decl.arg_size;
            _fdef.v.Coded.args = curr.v.function_decl.args;
            _fdef.v.Coded.body = curr.v.function_decl.body;
            _fdef.v.Coded.body_size = curr.v.function_decl.body_size;
            _fdef.parent = mod;

            if (curr.v.function_decl.takes_var_args)
                _fdef.arg_acceptance_count = -1;
            else if (curr.v.function_decl.takes_def_args)
                _fdef.arg_acceptance_count = -2;

            int f_idx = PSG_AddFunction(_fdef);
            expr_t retv = (expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = f_idx, .name = (char *)fname}, .line = curr.line};

            *RET = retv;

            if (fname != NULL)
            {
                _IPSF_AddVar_toModule(mod, (char *)fname, *RET);

                if (curr.v.function_decl.deco_inh != NULL)
                {
                    expr_t deco_red = IPSF_ReduceExpr_toConstant(mod, *(curr.v.function_decl.deco_inh));

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                    expr_t *exp = OSF_Malloc(sizeof(expr_t));
                    exp->type = EXPR_TYPE_FUNCTION_CALL;
                    exp->v.function_call.arg_size = 1;
                    exp->v.function_call.args = RET;
                    exp->v.function_call.is_func_call = 1;
                    exp->v.function_call.name = &deco_red;

                    stmt_t vd;
                    vd.type = STATEMENT_TYPE_VAR_DECL;
                    vd.v.var_decl.expr = exp;
                    vd.v.var_decl.name = &((expr_t){
                        .type = EXPR_TYPE_VARIABLE,
                        .v.variable.name = (char *)fname,
                        .line = curr.line});

                    IPSF_ExecVarDecl_fromStmt(mod, vd, NULL);
                    OSF_Free(exp);
                }
            }
        }
        break;
        case STATEMENT_TYPE_CLASS_DECL:
        {
            const char *cname = (const char *)curr.v.class_decl.name;

            if (cname != NULL) // Anonymous classes not allowed
            {
                class_t _cdef;
                _cdef.name = (char *)cname;
                _cdef.is_native = 0;
                _cdef.mod = SF_CreateModule(MODULE_TYPE_CLASS, NULL);
                _cdef.object_count = 0;
                _cdef.mod->parent = mod;
                _cdef.killed = 0;
                _cdef.objects = NULL;

                BODY(_cdef.mod)->body = curr.v.class_decl.body;
                BODY(_cdef.mod)->body_size = curr.v.class_decl.body_size;

                OSF_Free(IPSF_ExecIT_fromMod(_cdef.mod, err));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                if (err != NULL)
                    if (*err != IPSF_OK)
                        return NULL;

                int _cdef_idx = PSG_AddClass(_cdef);
                *RET = (expr_t){
                    .type = EXPR_TYPE_CLASS,
                    .v = {
                        .class_expr = {
                            .index = _cdef_idx}},
                    .line = curr.line};

                _IPSF_AddVar_toModule(mod, (char *)cname, *RET);
            }
        }
        break;
        case STATEMENT_TYPE_RETURN:
        {
            assert(mod->type == MODULE_TYPE_FUNCTION && SF_FMT("Error: `return' called outside function."));

            if (mod->returns == NULL)
                mod->returns = OSF_Malloc(sizeof(expr_t));

            *(mod->returns) = IPSF_ReduceExpr_toConstant(mod, *(curr.v.return_stmt.expr));
            doBreak = 1;
            break;
        }
        break;
        case STATEMENT_TYPE_IMPORT:
        {
            psf_byte_array_t *_ast;
            char *fd = (char *)read_file(curr.v.import_s.path);
            char *fn = OSF_GetFileName();
            char *pres_fd = OSF_GetFileData();

            OSF_SetFileData(fd);
            OSF_SetFileName(curr.v.import_s.path);

            _ast = PSF_AST_fromString(fd);
            PSF_AST_Preprocess_fromByteArray(_ast);

            mod_t *imod = SF_CreateModule(MODULE_TYPE_FILE, _ast);
            imod->path_prefix = _IPSF_GetDir_FromFilePath(curr.v.import_s.path);
            SF_FrameIT_fromAST(imod);
            BODY(imod)->name = curr.v.import_s.alias;

            SFBuiltIn_AddDefaultFunctions(imod);

            int _err;
            OSF_Free(IPSF_ExecIT_fromMod(imod, &_err));

            OSF_SetFileData(pres_fd);
            OSF_SetFileName(fn);

            int dump_import = !curr.v.import_s.arg_count;

            if (dump_import)
            {
                int md_idx = PSG_AddModule(imod);
                assert(curr.v.import_s.alias != NULL && SF_FMT("Error: Imports must have an alias"));

                const char *varname = (const char *)curr.v.import_s.alias;

                if (varname != NULL)
                {
                    _IPSF_AddVar_toModule(mod, (char *)varname, (expr_t){.type = EXPR_TYPE_MODULE, .v = {.module_s = {.index = md_idx}}, .line = curr.line});
                }
            }
            else
            {
                if (curr.v.import_s.alias == NULL)
                {
                    for (size_t j = 0; j < curr.v.import_s.arg_count; j++)
                    {
                        assert(
                            curr.v.import_s.args[j].type == EXPR_TYPE_VARIABLE &&
                            SF_FMT("Error: Syntax Error."));

                        var_t *g_v = IPSF_GetVar_fromMod(imod, curr.v.import_s.args[j].v.variable.name, NULL);

                        if (g_v == NULL)
                        {
                            OSF_SetException((except_t){
                                .type = EXCEPT_VARIABLE_DOES_NOT_EXIST,
                                .line = curr.line,
                                .v.ce1 = {
                                    .m_ref = imod,
                                    .vname = curr.v.import_s.args[j].v.variable.name}});

                            goto __label_abrupt_end_IPSF_ExecIT_fromMod;
                        }

                        _IPSF_AddVar_toModule(mod, g_v->name, g_v->val);
                    }
                }
                else
                {
                    mod_t *mmod = SF_ModuleCopy(imod);
                    // Do not free late reference
                    mmod->var_holds = OSF_Malloc(sizeof(var_t));
                    mmod->var_holds_size = 0;

                    for (size_t j = 0; j < curr.v.import_s.arg_count; j++)
                    {
                        assert(
                            curr.v.import_s.args[j].type == EXPR_TYPE_VARIABLE &&
                            SF_FMT("Error: Syntax Error."));

                        var_t *g_v = IPSF_GetVar_fromMod(imod, curr.v.import_s.args[j].v.variable.name, NULL);

                        if (g_v == NULL)
                        {
                            OSF_SetException((except_t){
                                .type = EXCEPT_VARIABLE_DOES_NOT_EXIST,
                                .line = curr.line,
                                .v.ce1 = {
                                    .m_ref = imod,
                                    .vname = curr.v.import_s.args[j].v.variable.name}});

                            goto __label_abrupt_end_IPSF_ExecIT_fromMod;
                        }

                        _IPSF_AddVar_toModule(mmod, g_v->name, g_v->val);
                    }

                    int md_idx = PSG_AddModule(mmod);

                    const char *varname = (const char *)curr.v.import_s.alias;

                    _IPSF_AddVar_toModule(mod, (char *)varname, (expr_t){.type = EXPR_TYPE_MODULE, .v = {.module_s = {.index = md_idx}}, .line = curr.line});
                }
            }
        }
        break;
        case STATEMENT_TYPE_SWITCH:
        {
            expr_t red_cond = IPSF_ReduceExpr_toConstant(mod, *(curr.v.switch_case.condition));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            int c_true = 0;

            for (size_t j = 0; j < curr.v.switch_case.cases_count; j++)
            {
                expr_t curr_case_red = IPSF_ReduceExpr_toConstant(mod, *(curr.v.switch_case.cases[j].condition));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecIT_fromMod;

                // printf("[%s]\n", curr.v.switch_case.cases[i].condition->v.constant.String.value);
                // PSG_PrintExprType(red_cond.type);
                // PSG_PrintExprType(curr_case_red.type);
                if (!curr.v.switch_case.cases[j].is_case_in)
                {
                    if (_PSF_EntityIsTrue(_IPSF_ExecLogicalArithmetic(red_cond, LOGICAL_OP_EQEQ, curr_case_red)))
                    {
                        // mod_t *m = SF_CreateModule(mod->type, NULL);
                        // m->parent = mod;
                        // BODY(m)->body = curr.v.switch_case.cases[j].body;
                        // BODY(m)->body_size = curr.v.switch_case.cases[j].body_size;

                        // OSF_Free(IPSF_ExecIT_fromMod(m, NULL));

                        psf_byte_array_t *pres_ast = mod->ast;
                        stmt_t *pres_body = BODY(mod)->body;
                        int pres_size = BODY(mod)->body_size;
                        int pres_type = mod->type;

                        BODY(mod)->body = curr.v.switch_case.cases[j].body;
                        BODY(mod)->body_size = curr.v.switch_case.cases[j].body_size;

                        OSF_Free(IPSF_ExecIT_fromMod(mod, NULL));

                        mod->type = pres_type;
                        mod->ast = pres_ast;
                        BODY(mod)->body = pres_body;
                        BODY(mod)->body_size = pres_size;

                        c_true = 1;
                        break;
                    }
                }
                else
                {
                    if (_PSF_EntityIsTrue(curr_case_red))
                    {
                        // mod_t *m = SF_CreateModule(mod->type, NULL);
                        // m->parent = mod;
                        // BODY(m)->body = curr.v.switch_case.cases[j].body;
                        // BODY(m)->body_size = curr.v.switch_case.cases[j].body_size;

                        // OSF_Free(IPSF_ExecIT_fromMod(m, NULL));

                        psf_byte_array_t *pres_ast = mod->ast;
                        stmt_t *pres_body = BODY(mod)->body;
                        int pres_size = BODY(mod)->body_size;
                        int pres_type = mod->type;

                        BODY(mod)->body = curr.v.switch_case.cases[j].body;
                        BODY(mod)->body_size = curr.v.switch_case.cases[j].body_size;

                        OSF_Free(IPSF_ExecIT_fromMod(mod, NULL));

                        mod->type = pres_type;
                        mod->ast = pres_ast;
                        BODY(mod)->body = pres_body;
                        BODY(mod)->body_size = pres_size;

                        c_true = 1;
                        break;
                    }
                }
            }

            if (!c_true)
            {
                // Execute default case, if provided
                if (curr.v.switch_case.def_case.body_size)
                {
                    // mod_t *m = SF_CreateModule(mod->type, NULL);
                    // m->parent = mod;
                    // BODY(m)->body = curr.v.switch_case.def_case.body;
                    // BODY(m)->body_size = curr.v.switch_case.def_case.body_size;

                    // OSF_Free(IPSF_ExecIT_fromMod(m, NULL));

                    psf_byte_array_t *pres_ast = mod->ast;
                    stmt_t *pres_body = BODY(mod)->body;
                    int pres_size = BODY(mod)->body_size;
                    int pres_type = mod->type;

                    BODY(mod)->body = curr.v.switch_case.def_case.body;
                    BODY(mod)->body_size = curr.v.switch_case.def_case.body_size;

                    OSF_Free(IPSF_ExecIT_fromMod(mod, NULL));

                    mod->type = pres_type;
                    mod->ast = pres_ast;
                    BODY(mod)->body = pres_body;
                    BODY(mod)->body_size = pres_size;
                }
            }
        }
        break;
        case STATEMENT_TYPE_ASSERT:
        {
            expr_t _c_red = IPSF_ReduceExpr_toConstant(mod, *(curr.v.assert_stmt.condition));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecIT_fromMod;

            if (!_PSF_EntityIsTrue(_c_red))
            {
                if (curr.v.assert_stmt.message != NULL)
                {
                    expr_t _m_red = IPSF_ReduceExpr_toConstant(mod, *(curr.v.assert_stmt.message));
                    printf("%s\n", _IPSF_ObjectRepr(_m_red, 0));
                }
                else
                {
                    assert(0 && SF_FMT("Error: Assertion Error."));
                }

                exit(0);
            }
        }
        break;

        default:
            break;
        }

        if (doBreak)
            break;

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecIT_fromMod;

        // fflush(stdout);
        // fflush(stderr);
        // fflush(stdin);

        OSF_PopBackLog();
    }

__label_abrupt_end_IPSF_ExecIT_fromMod:

    if (OSF_GetExceptionState())
        if (err != NULL)
            *err = IPSF_NOT_OK_CHECK_EXPR_LOG;

    return RET;
}

expr_t *IPSF_ExecExprStatement_fromMod(mod_t *mod, stmt_t stmt, int *err)
{
    expr_t *expr = stmt.v.expr.expr,
           *RES = (expr_t *)OSF_Malloc(sizeof(expr_t));

    if (expr == NULL)
        exit(printf("Error: seg_fault\n"));

    expr->line = stmt.line;
    OSF_AddBackLog(OSF_CreateBackLog(BACKTRACE_ENTITY_EXPR, expr->line));

    switch (expr->type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        expr_t ex_cpy = *expr;

        switch (ex_cpy.v.constant.constant_type)
        {
        case CONSTANT_TYPE_ARRAY:
        {
            // DO NOT FREE
            if (!ARRAY(expr->v.constant.Array.index).evaluated)
            {
                expr_t *cpy_vals = ARRAY(expr->v.constant.Array.index).vals;
                expr_t *cpy_reds = OSF_Malloc(ARRAY(expr->v.constant.Array.index).len * sizeof(expr_t));

                for (size_t j = 0; j < ARRAY(expr->v.constant.Array.index).len; j++)
                {
                    cpy_reds[j] = IPSF_ReduceExpr_toConstant(mod, cpy_vals[j]);

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
                }

                ex_cpy.v.constant.Array.index = PSG_AddArray(Sf_Array_New_fromExpr(cpy_reds, ARRAY(expr->v.constant.Array.index).len));

                PSG_GetArray_Ptr(ex_cpy.v.constant.Array.index)->evaluated = 1;
            }
        }
        break;
        case CONSTANT_TYPE_DICT:
        {
            if (!DICT(expr->v.constant.Dict.index).evaluated)
            {
                expr_t *cpy_keys = DICT(expr->v.constant.Dict.index).keys;
                expr_t *cpy_reds_k = OSF_Malloc(DICT(expr->v.constant.Dict.index).len * sizeof(expr_t));

                for (size_t j = 0; j < DICT(expr->v.constant.Dict.index).len; j++)
                {
                    cpy_reds_k[j] = IPSF_ReduceExpr_toConstant(mod, cpy_keys[j]);

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
                }

                expr_t *cpy_vals = DICT(expr->v.constant.Dict.index).vals;
                expr_t *cpy_reds = OSF_Malloc(DICT(expr->v.constant.Dict.index).len * sizeof(expr_t));

                for (size_t j = 0; j < DICT(expr->v.constant.Dict.index).len; j++)
                {
                    cpy_reds[j] = IPSF_ReduceExpr_toConstant(mod, cpy_vals[j]);

                    if (OSF_GetExceptionState())
                        goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
                }

                ex_cpy.v.constant.Dict.index = PSG_AddDict(Sf_Dict_New_fromExpr(cpy_reds_k, cpy_reds, DICT(expr->v.constant.Dict.index).len));

                PSG_GetDict_Ptr(ex_cpy.v.constant.Dict.index)->evaluated = 1;
            }
        }
        break;

        default:
            break;
        }
        *RES = ex_cpy;
    }
    break;
    case EXPR_TYPE_FUNCTION_CALL:
    {
        // printf("[x]"); OSF_PrintHeapSize(); printf("\n");

        if (!expr->v.function_call.is_func_call)
        {
            // Normal expression call in Parser scope.
            // Handle it that way

            for (size_t j = 0; j < expr->v.function_call.arg_size; j++)
            {
                expr_t temp = IPSF_ReduceExpr_toConstant(mod, expr->v.function_call.args[j]);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            }
            break;
        }

        expr_t _red_nme = IPSF_ReduceExpr_toConstant(mod, *(expr->v.function_call.name));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        switch (_red_nme.type)
        {
        case EXPR_TYPE_FUNCTION:
        {
            int *gfsz = PSG_GetFunctionsSize();
            fun_t **gffs = PSG_GetFunctions();

            expr_t get_f = _red_nme;
            fun_t fun_s = (*gffs)[get_f.v.function_s.index];
            mod_t *m_pass = fun_s.parent;

            if (get_f.v.function_s.index >= *gfsz)
                exit(printf("Error: seg_fault\n"));

            expr_t *f_args = OSF_Malloc((expr->v.function_call.arg_size ? expr->v.function_call.arg_size : 1) * sizeof(expr_t));
            int fa_beg = 0;

            if (OSF_MemorySafeToFree(get_f.next)) // If it was OSF_Mallocated, this condition would be true
            {
                if (get_f.next->type != EXPR_TYPE_MODULE)
                {
                    f_args = OSF_Realloc(f_args, (expr->v.function_call.arg_size + 1) * sizeof(expr_t));
                    f_args[fa_beg++] = *(get_f.next);

                    switch (get_f.next->type)
                    {
                    case EXPR_TYPE_CONSTANT:
                    {
                        switch (get_f.next->v.constant.constant_type)
                        {
                        case CONSTANT_TYPE_CLASS_OBJECT:
                        {
                            m_pass = _IPSF_GetClass_fromIntTuple(get_f.next->v.constant.ClassObj.idx)->mod;
                        }
                        break;

                        default:
                            break;
                        }
                    }
                    break;

                    default:
                        break;
                    }
                }
                else
                    m_pass = PSG_GetModule(get_f.next->v.module_s.index);
            }

            for (size_t j = 0; j < expr->v.function_call.arg_size; j++)
            {
                if (expr->v.function_call.args[j].type == EXPR_TYPE_INLINE_ASSIGNMENT)
                {
                    f_args[j + fa_beg] = expr->v.function_call.args[j];
                    f_args[j + fa_beg].v.inline_assignment.rhs = OSF_Malloc(sizeof(expr_t));
                    *(f_args[j + fa_beg].v.inline_assignment.rhs) = IPSF_ReduceExpr_toConstant(mod, *(expr->v.function_call.args[j].v.inline_assignment.rhs));
                }
                else
                    f_args[j + fa_beg] = IPSF_ReduceExpr_toConstant(mod, expr->v.function_call.args[j]);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            }
            fa_beg += expr->v.function_call.arg_size;

            expr_t *f_res = _IPSF_CallFunction(fun_s, f_args, fa_beg, m_pass);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

            *RES = *f_res;
            OSF_Free(f_res);

            OSF_Free(f_args);
        }
        // printf("[y]"); OSF_PrintHeapSize(); printf("\n");
        break;
        case EXPR_TYPE_CLASS:
        {
            expr_t get_c = _red_nme;
            if (get_c.v.class_expr.index >= *PSG_GetClassesSize())
                exit(printf("Error: seg_fault\n"));

            (*PSG_GetClasses())[get_c.v.class_expr.index].object_count++;
            class_t cref = (*PSG_GetClasses())[get_c.v.class_expr.index];

            /**
             * @brief Step 1:
             * Create and register the instance
             */
            class_t inst_c;
            inst_c.is_native = cref.is_native;
            inst_c.mod = SF_ModuleCopy(cref.mod);
            inst_c.name = OSF_strdup(cref.name);
            inst_c.object_count = 0;
            inst_c.killed = 0;

            int_tuple cobj_idx = PSG_AddClassObject_toClass(inst_c, get_c.v.class_expr.index);

            class_t *inst_ref = &((*PSG_GetClasses())[cobj_idx.x1].objects[cobj_idx.x2]);

            int ve = 0;
            var_t *var__main = IPSF_GetVar_fromMod(inst_ref->mod, "__main__", &ve);

            assert(ve != IPSF_ERR_VAR_NOT_FOUND && SF_FMT("Class has no constructor."));
            assert(var__main->val.type == EXPR_TYPE_FUNCTION && SF_FMT("`__main__' not a function"));

            expr_t *cons_args = OSF_Malloc((expr->v.function_call.arg_size + 1) * sizeof(expr_t));
            *cons_args = (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v = {
                    .constant = {
                        .constant_type = CONSTANT_TYPE_CLASS_OBJECT,
                        .ClassObj = {
                            .idx = cobj_idx}}},
                .line = expr->line};

            for (size_t j = 0; j < expr->v.function_call.arg_size; j++)
            {
                if (expr->v.function_call.args[j].type == EXPR_TYPE_INLINE_ASSIGNMENT)
                {
                    cons_args[j + 1] = expr->v.function_call.args[j];
                    cons_args[j + 1].v.inline_assignment.rhs = OSF_Malloc(sizeof(expr_t));
                    *(cons_args[j + 1].v.inline_assignment.rhs) = IPSF_ReduceExpr_toConstant(mod, *(expr->v.function_call.args[j].v.inline_assignment.rhs));
                }
                else
                    cons_args[j + 1] = IPSF_ReduceExpr_toConstant(mod, expr->v.function_call.args[j]);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            }

            mod_t *mpass = inst_c.mod->parent;
            fun_t _c_main = (*PSG_GetFunctions())[var__main->val.v.function_s.index];
            expr_t *main_res = _IPSF_CallFunction(_c_main, cons_args, expr->v.function_call.arg_size + 1, mpass);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

            *RES = *cons_args; // self

            OSF_Free(cons_args);
            cons_args = NULL;
        }
        break;
        case EXPR_TYPE_MODULE:
        {
            expr_t *f_args = OSF_Malloc(expr->v.function_call.arg_size * sizeof(expr_t));
            int f_arg_size = 0;
            mod_t *m_ref = PSG_GetModule(_red_nme.v.module_s.index);
            mod_t *pres_mref_p = m_ref->parent;

            var_t *get_main = IPSF_GetVar_fromMod(m_ref, "__main__", NULL);
            assert(get_main != NULL && SF_FMT("Error: Module has no constructor."));
            assert(get_main->val.type == EXPR_TYPE_FUNCTION && SF_FMT("Error: Module constructor is not a function."));

            m_ref->parent = mod;

            for (size_t j = 0; j < expr->v.function_call.arg_size; j++)
            {
                assert(expr->v.function_call.args[j].type != EXPR_TYPE_INLINE_ASSIGNMENT && SF_FMT("Error: Inline assignment is not allowed in module constructors."));
                expr_t rd = IPSF_ReduceExpr_toConstant(mod, expr->v.function_call.args[j]);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
                if (!_IPSF_IsDataType_Void(rd))
                    f_args[f_arg_size++] = rd;
            }

            m_ref->parent = pres_mref_p;

            fun_t main_func = (*PSG_GetFunctions())[get_main->val.v.function_s.index];
            expr_t *mf_res = _IPSF_CallFunction(main_func, f_args, f_arg_size, m_ref);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

            mf_res->next = OSF_Malloc(sizeof(expr_t));
            *(mf_res->next) = _red_nme;

            *RES = *mf_res;

            OSF_Free(mf_res);
            OSF_Free(f_args);
        }
        break;

        default:
            break;
        }
    }
    break;
    case EXPR_TYPE_VARIABLE:
    {
        int err_c = 0;
        var_t *pvv = IPSF_GetVar_fromMod(mod, expr->v.variable.name, &err_c);

        // if (err_c == IPSF_ERR_VAR_NOT_FOUND)
        //     printf("varname[%s]\n", expr->v.variable.name);

        if (pvv == NULL)
        {
            OSF_SetException((except_t){
                .type = EXCEPT_VARIABLE_DOES_NOT_EXIST,
                .line = expr->line,
                .v.ce1 = {
                    .m_ref = mod,
                    .vname = expr->v.variable.name}});

            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
        }

        *RES = pvv->val;
    }
    break;
    case EXPR_TYPE_INLINE_ASSIGNMENT:
    {
        expr_t *lhs = expr->v.inline_assignment.lhs,
               *rhs = expr->v.inline_assignment.rhs;

        if (lhs->type == EXPR_TYPE_VARIABLE)
        {
            struct __mod_child_varhold_s *lhs_val = IPSF_GetVar_fromMod(mod, lhs->v.variable.name, err);

            if (lhs_val == NULL)
            {
                OSF_SetException((except_t){
                    .type = EXCEPT_VARIABLE_DOES_NOT_EXIST,
                    .line = expr->line,
                    .v.ce1 = {
                        .m_ref = mod,
                        .vname = lhs->v.variable.name}});

                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            }

            (*lhs_val).val = IPSF_ReduceExpr_toConstant(mod, *rhs);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            *RES = (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v = {
                    .constant = {
                        .constant_type = CONSTANT_TYPE_DTYPE,
                        .DType = {
                            .type = DATA_TYPE_VOID}}},
                .line = expr->line};
        }
        else
        {
            // TODO: here
            *RES = IPSF_ReduceExpr_toConstant(mod, *rhs);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
        }
    }
    break;
    case EXPR_TYPE_LOGICAL_ARITHMETIC_OP:
    {
        int op_ty = expr->v.logical_arith_op_expr.op;
        expr_t _lhs = IPSF_ReduceExpr_toConstant(mod, *(expr->v.logical_arith_op_expr.lhs)),
               _rhs = IPSF_ReduceExpr_toConstant(mod, *(expr->v.logical_arith_op_expr.rhs));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        *RES = _IPSF_ExecLogicalArithmetic(_lhs, op_ty, _rhs);
    }
    break;
    case EXPR_TYPE_TO_STEP_CLAUSE:
    {
        expr_t sres;
        // Array
        sres.type = EXPR_TYPE_CONSTANT;
        sres.v.constant.constant_type = CONSTANT_TYPE_ARRAY;
        sres.v.constant.Array.index = PSG_AddArray(Sf_Array_New());

        int has_step = expr->v.to_step_clause._step != NULL;
        int step_count = 1;

        if (has_step)
        {
            expr_t _step_red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.to_step_clause._step));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            assert(
                (_step_red.type == EXPR_TYPE_CONSTANT &&
                 _step_red.v.constant.constant_type == CONSTANT_TYPE_INT) &&
                SF_FMT("Error: step count must be an integer"));

            step_count = _step_red.v.constant.Int.value;
        }

        expr_t lhs_red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.to_step_clause.lhs)),
               rhs_red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.to_step_clause.rhs));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        assert(
            ((
                 lhs_red.type == EXPR_TYPE_CONSTANT &&
                 lhs_red.v.constant.constant_type == CONSTANT_TYPE_INT) &&
             (rhs_red.type == EXPR_TYPE_CONSTANT &&
              rhs_red.v.constant.constant_type == CONSTANT_TYPE_INT) &&
             SF_FMT("Error: Entities for to..step clause must be integers")));

        for (int j = lhs_red.v.constant.Int.value;
             j < rhs_red.v.constant.Int.value;
             j += step_count)
        {
            if (ARRAY(sres.v.constant.Array.index).len)
                ARRAY(sres.v.constant.Array.index).vals = OSF_Realloc(
                    ARRAY(sres.v.constant.Array.index).vals,
                    (ARRAY(sres.v.constant.Array.index).len + 1) *
                        sizeof(expr_t));
            ARRAY(sres.v.constant.Array.index).vals[(PSG_GetArray_Ptr(sres.v.constant.Array.index)->len)++] = (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v.constant = {
                    .constant_type = CONSTANT_TYPE_INT,
                    .Int.value = j},
                .line = expr->line};
        }

        *RES = sres;
    }
    break;
    case EXPR_TYPE_INDEX_OP:
    {
        expr_t _entity_red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.index_op.entity)),
               _index_red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.index_op.index));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        expr_t _fres;
        _fres.line = expr->line;

        switch (_entity_red.v.constant.constant_type)
        {
        case CONSTANT_TYPE_ARRAY:
        {
            if (_index_red.v.constant.Int.value >= ARRAY(_entity_red.v.constant.Array.index).len)
            {
                OSF_SetException((except_t){
                    .type = EXCEPT_INDEX_OUT_OF_RANGE,
                    .line = _entity_red.line,
                    .v.ce0 = {
                        .idx = _index_red.v.constant.Int.value,
                        .targ = ARRAY(_entity_red.v.constant.Array.index)}});

                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            }

            if (_index_red.v.constant.Int.value < 0)
                _index_red.v.constant.Int.value +=
                    ARRAY(_entity_red.v.constant.Array.index).len;

            _fres = ARRAY(_entity_red.v.constant.Array.index).vals[_index_red.v.constant.Int.value];
        }
        break;
        case CONSTANT_TYPE_DICT:
        {
            dict_t gd = DICT(_entity_red.v.constant.Dict.index);
            _fres = Sf_Dict_Get_fromKey(&gd, _index_red);
        }
        break;
        case CONSTANT_TYPE_CLASS_OBJECT:
        {
            class_t *cobj = _IPSF_GetClass_fromIntTuple(_entity_red.v.constant.ClassObj.idx);
            var_t *get_operator_index = IPSF_GetVar_fromMod(cobj->mod, "operator[]", NULL);

            assert(
                get_operator_index != NULL &&
                SF_FMT("Error: Class object cannot be indexed"));

            assert(
                get_operator_index->val.type == EXPR_TYPE_FUNCTION &&
                SF_FMT("Error: `operator[]' must be a function"));

            fun_t _f = (*PSG_GetFunctions())[get_operator_index->val.v.function_s.index];
            expr_t *_f_res = _IPSF_CallFunction(_f, (expr_t *)(expr_t[]){_entity_red, _index_red}, 2, cobj->mod->parent);

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

            _fres = *_f_res;

            OSF_Free(_f_res);
        }
        break;

        default:
            break;
        }

        *RES = _fres;
    }
    break;
    case EXPR_TYPE_INLINE_FOR:
    {
        expr_t res;
        res.type = EXPR_TYPE_CONSTANT;
        res.v.constant.constant_type = CONSTANT_TYPE_ARRAY;
        res.v.constant.Array.index = PSG_AddArray(Sf_Array_New());

        expr_t cond_red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.inline_for.condition));
        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        mod_t *f_mod = SF_CreateModule(mod->type, NULL);
        f_mod->parent = mod;
        OSF_Free(BODY(f_mod)->body);
        var_t *expr_vars = (var_t *)expr->v.inline_for.vars;

        switch (cond_red.v.constant.constant_type)
        {
        case CONSTANT_TYPE_ARRAY:
        {
            for (size_t j = 0; j < ARRAY(cond_red.v.constant.Array.index).len; j++)
            {
                expr_t idx_val = ARRAY(cond_red.v.constant.Array.index).vals[j];

                switch (idx_val.v.constant.constant_type)
                {
                case CONSTANT_TYPE_ARRAY:
                {
                    expr_t *arr_ref = ARRAY(idx_val.v.constant.Array.index).vals;
                    int sz_ref = ARRAY(idx_val.v.constant.Array.index).len;

                    if (expr->v.inline_for.var_size == sz_ref)
                    {
                        for (size_t k = 0; k < sz_ref; k++)
                        {
                            _IPSF_AddVar_toModule(
                                f_mod,
                                expr_vars[k].name,
                                arr_ref[k]);
                        }
                    }
                    else if (expr->v.inline_for.var_size < sz_ref)
                    {
                        for (size_t k = 0; k < expr->v.inline_for.var_size; k++)
                        {
                            _IPSF_AddVar_toModule(
                                f_mod,
                                expr_vars[k].name,
                                arr_ref[k]);
                        }
                    }
                    else if (expr->v.inline_for.var_size > sz_ref)
                    {
                        for (size_t k = 0; k < sz_ref; k++)
                        {
                            _IPSF_AddVar_toModule(
                                f_mod,
                                expr_vars[k].name,
                                arr_ref[k]);
                        }
                    }
                }
                break;

                default:
                {
                    _IPSF_AddVar_toModule(
                        f_mod,
                        expr_vars[0].name,
                        idx_val);
                }
                break;
                }

                // Add variables as a check afterhand
                for (size_t k = 0; k < expr->v.inline_for.var_size; k++)
                {
                    int err_c = IPSF_OK;
                    var_t *pv_v = IPSF_GetVar_fromMod(f_mod, expr_vars[k].name, &err_c);

                    /**
                     * @brief Variables who could not fit to data
                     * will not be declared in the module.
                     * But ignoring them as symbols is unethical if such symbols
                     * have a default value.
                     * So, they will be added here ONLY IF they have a default value
                     */
                    if (err_c == IPSF_ERR_VAR_NOT_FOUND)
                    {
                        expr_t _vred = IPSF_ReduceExpr_toConstant(f_mod, expr_vars[k].val);

                        if (OSF_GetExceptionState())
                            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

                        if (!_IPSF_IsDataType_Void(_vred))
                            _IPSF_AddVar_toModule(
                                f_mod,
                                expr_vars[k].name,
                                _vred);

                        continue;
                    }
                }

                expr_t _ex_res = IPSF_ReduceExpr_toConstant(f_mod, *(expr->v.inline_for.body));

                if (ARRAY(res.v.constant.Array.index).len)
                    ARRAY(res.v.constant.Array.index).vals = OSF_Realloc(
                        ARRAY(res.v.constant.Array.index).vals,
                        (ARRAY(res.v.constant.Array.index).len + 1) *
                            sizeof(expr_t));

                ARRAY(res.v.constant.Array.index).vals[(PSG_GetArray_Ptr(res.v.constant.Array.index)->len)++] = _ex_res;

                SF_Module_safeDelete(f_mod);
                f_mod->var_holds = OSF_Malloc(sizeof(var_t));
                f_mod->var_holds_size = 0;
            }
        }
        break;
        case CONSTANT_TYPE_CLASS_OBJECT:
        {
            class_t *_c_ref = _IPSF_GetClass_fromIntTuple(cond_red.v.constant.ClassObj.idx);
            var_t *iter_conf = IPSF_GetVar_fromMod(_c_ref->mod, "operatorfor..in", NULL);
            assert(iter_conf != NULL && SF_FMT("Error: Class object is not an iterable."));
            assert(iter_conf->val.type == EXPR_TYPE_FUNCTION && SF_FMT("Error: `operator for..in' is not a function."));
            fun_t _get_iter = (*PSG_GetFunctions())[iter_conf->val.v.function_s.index];

            expr_t *_itc_val = _IPSF_CallFunction(_get_iter, (expr_t *)(expr_t[]){cond_red}, 1, _c_ref->mod->parent);
            while (!_IPSF_IsDataType_None(*_itc_val))
            {
                expr_t idx_val = *_itc_val;
                assert((
                           idx_val.type == EXPR_TYPE_CONSTANT &&
                           idx_val.v.constant.constant_type == CONSTANT_TYPE_CLASS_OBJECT) &&
                       SF_FMT("Error: Iterative must be a class object."));

                class_t *c_get = _IPSF_GetClass_fromIntTuple(idx_val.v.constant.ClassObj.idx);
                var_t *get_iter_repr = IPSF_GetVar_fromMod(c_get->mod, "__iter_repr__", NULL);
                assert(get_iter_repr != NULL && SF_FMT("Error: Class object is not an iterative."));
                assert(get_iter_repr->val.type == EXPR_TYPE_FUNCTION && SF_FMT("Error: `__iter_repr__' is not a function."));
                fun_t _get_iter_repr = (*PSG_GetFunctions())[get_iter_repr->val.v.function_s.index];

                expr_t *idx_v_dy = _IPSF_CallFunction(_get_iter_repr, (expr_t *)(expr_t[]){idx_val}, 1, c_get->mod->parent);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
                idx_val = *idx_v_dy;
                OSF_Free(idx_v_dy);

                switch (idx_val.v.constant.constant_type)
                {
                case CONSTANT_TYPE_ARRAY:
                {
                    expr_t *arr_ref = ARRAY(idx_val.v.constant.Array.index).vals;
                    int sz_ref = ARRAY(idx_val.v.constant.Array.index).len;

                    if (expr->v.inline_for.var_size == sz_ref)
                    {
                        for (size_t k = 0; k < sz_ref; k++)
                        {
                            _IPSF_AddVar_toModule(
                                f_mod,
                                ((var_t *)expr->v.inline_for.vars)[k].name,
                                arr_ref[k]);
                        }
                    }
                    else if (expr->v.inline_for.var_size < sz_ref)
                    {
                        for (size_t k = 0; k < expr->v.inline_for.var_size; k++)
                        {
                            _IPSF_AddVar_toModule(
                                f_mod,
                                ((var_t *)expr->v.inline_for.vars)[k].name,
                                arr_ref[k]);
                        }
                    }
                    else if (expr->v.inline_for.var_size > sz_ref)
                    {
                        for (size_t k = 0; k < sz_ref; k++)
                        {
                            _IPSF_AddVar_toModule(
                                f_mod,
                                ((var_t *)expr->v.inline_for.vars)[k].name,
                                arr_ref[k]);
                        }
                    }
                }
                break;

                default:
                {
                    _IPSF_AddVar_toModule(
                        f_mod,
                        ((var_t *)expr->v.inline_for.vars)[0].name,
                        idx_val);
                }
                break;
                }

                // Add variables as a check afterhand
                for (size_t k = 0; k < expr->v.inline_for.var_size; k++)
                {
                    int err_c = IPSF_OK;
                    var_t *pv_v = IPSF_GetVar_fromMod(f_mod, ((var_t *)expr->v.inline_for.vars)[k].name, &err_c);

                    /**
                     * @brief Variables who could not fit to data
                     * will not be declared in the module.
                     * But ignoring them as symbols is unethical if such symbols
                     * have a default value.
                     * So, they will be added here ONLY IF they have a default value
                     */
                    if (err_c == IPSF_ERR_VAR_NOT_FOUND)
                    {
                        expr_t _vred = IPSF_ReduceExpr_toConstant(f_mod, ((var_t *)expr->v.inline_for.vars)[k].val);

                        if (OSF_GetExceptionState())
                            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

                        if (!_IPSF_IsDataType_Void(_vred))
                            _IPSF_AddVar_toModule(
                                f_mod,
                                ((var_t *)expr->v.inline_for.vars)[k].name,
                                _vred);

                        continue;
                    }
                }

                expr_t _ex_res = IPSF_ReduceExpr_toConstant(f_mod, *(expr->v.inline_for.body));

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

                if (ARRAY(res.v.constant.Array.index).len)
                    ARRAY(res.v.constant.Array.index).vals = OSF_Realloc(
                        ARRAY(res.v.constant.Array.index).vals,
                        (ARRAY(res.v.constant.Array.index).len + 1) *
                            sizeof(expr_t));

                ARRAY(res.v.constant.Array.index).vals[(PSG_GetArray_Ptr(res.v.constant.Array.index)->len)++] = _ex_res;

                SF_Module_safeDelete(f_mod);
                f_mod->var_holds = OSF_Malloc(sizeof(var_t));
                f_mod->var_holds_size = 0;

                OSF_Free(_itc_val);
                _itc_val = _IPSF_CallFunction(_get_iter, (expr_t *)(expr_t[]){cond_red}, 1, _c_ref->mod->parent);

                if (OSF_GetExceptionState())
                    goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
            }

            OSF_Free(_itc_val);
        }
        break;
        default:
            break;
        }

        SF_Module_safeDelete(f_mod);
        OSF_Free(f_mod);

        *RES = res;
    }
    break;
    case EXPR_TYPE_THEN_WHILE:
    {
        expr_t res;
        res.type = EXPR_TYPE_CONSTANT;
        res.v.constant.constant_type = CONSTANT_TYPE_ARRAY;
        res.v.constant.Array.index = PSG_AddArray(Sf_Array_New());

        mod_t *w_mod = SF_CreateModule(mod->type, NULL);
        w_mod->parent = mod;
        OSF_Free(BODY(w_mod)->body);
        var_t *wexpr_vars = (var_t *)expr->v.then_while.withs;
        int wev_size = expr->v.then_while.withs_size;
        var_t *texpr_vars = (var_t *)expr->v.then_while.assigns;
        int tev_size = expr->v.then_while.assigns_size;

        for (size_t j = 0; j < wev_size; j++)
        {
            _IPSF_AddVar_toModule(
                w_mod,
                wexpr_vars[j].name,
                wexpr_vars[j].val);
        }

        expr_t cond_red = IPSF_ReduceExpr_toConstant(w_mod, *(expr->v.then_while.condition));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        while (_PSF_EntityIsTrue(cond_red))
        {
            expr_t _eres = IPSF_ReduceExpr_toConstant(w_mod, *(expr->v.then_while.body));

            if (ARRAY(res.v.constant.Array.index).len)
                ARRAY(res.v.constant.Array.index).vals = OSF_Realloc(ARRAY(res.v.constant.Array.index).vals, (ARRAY(res.v.constant.Array.index).len + 1) * sizeof(expr_t));

            ARRAY(res.v.constant.Array.index).vals[(PSG_GetArray_Ptr(res.v.constant.Array.index)->len)++] = _eres;

            for (size_t j = 0; j < tev_size; j++)
            {
                _IPSF_AddVar_toModule(
                    w_mod,
                    texpr_vars[j].name,
                    texpr_vars[j].val);
            }

            cond_red = IPSF_ReduceExpr_toConstant(w_mod, *(expr->v.then_while.condition));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
        }

        SF_Module_safeDelete(w_mod);
        OSF_Free(w_mod);

        *RES = res;
    }
    break;
    case EXPR_TYPE_INLINE_REPEAT:
    {
        expr_t cond = IPSF_ReduceExpr_toConstant(mod, *(expr->v.inline_repeat.cond));
        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
        expr_t res;

        res.type = EXPR_TYPE_CONSTANT;
        res.v.constant.constant_type = CONSTANT_TYPE_ARRAY;
        res.v.constant.Array.index = PSG_AddArray(Sf_Array_New());

        assert(
            (
                cond.type == EXPR_TYPE_CONSTANT &&
                cond.v.constant.constant_type == CONSTANT_TYPE_INT) &&
            SF_FMT("Error: Iteration count must be an integer."));

        int cond_iter = cond.v.constant.Int.value;

        while (cond_iter--)
        {
            expr_t _res = IPSF_ReduceExpr_toConstant(mod, *(expr->v.inline_repeat.body));

            if (OSF_GetExceptionState())
                goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

            if (ARRAY(res.v.constant.Array.index).len)
                ARRAY(res.v.constant.Array.index).vals = OSF_Realloc(ARRAY(res.v.constant.Array.index).vals, (ARRAY(res.v.constant.Array.index).len + 1) * sizeof(expr_t));

            ARRAY(res.v.constant.Array.index).vals[(PSG_GetArray_Ptr(res.v.constant.Array.index)->len)++] = _res;
        }

        *RES = res;
    }
    break;
    case EXPR_TYPE_UNARY_ARITHMETIC_OP:
    {
        expr_t _res = _IPSF_ExecUnaryArithmetic(mod, expr);

        *RES = _res;
    }
    break;
    case EXPR_TYPE_MEMBER_ACCESS:
    {
        expr_t _red = IPSF_ReduceExpr_toConstant(mod, *(expr->v.member_access.parent));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;
        expr_t _res;

        switch (_red.type)
        {
        case EXPR_TYPE_CLASS:
        {
            int ef = IPSF_OK;
            var_t *_v_ref = IPSF_GetVar_fromMod((*PSG_GetClasses())[_red.v.class_expr.index].mod, expr->v.member_access.child, &ef);

            assert(ef != IPSF_ERR_VAR_NOT_FOUND && SF_FMT("Error: Object has no member %s."));
            _res = _v_ref->val;
        }
        break;
        case EXPR_TYPE_CONSTANT:
        {
            if (_red.v.constant.constant_type == CONSTANT_TYPE_CLASS_OBJECT)
            {
                int ef = IPSF_OK;
                class_t *__c = _IPSF_GetClass_fromIntTuple(_red.v.constant.ClassObj.idx);
                mod_t *_pres_c_mod_parent = __c->mod->parent;
                __c->mod->parent = NULL;
                var_t *_v_ref = IPSF_GetVar_fromMod(__c->mod, expr->v.member_access.child, &ef);
                __c->mod->parent = _pres_c_mod_parent;

                assert(ef != IPSF_ERR_VAR_NOT_FOUND && SF_FMT("Error: Object has no member %s."));
                _res = _v_ref->val;
                _res.next = OSF_Malloc(sizeof(expr_t));
                *(_res.next) = _red;
            }
            else
            {
                var_t *gv = GetDtypePrototype_fromSymbolAndType(_red.v.constant.constant_type, expr->v.member_access.child);

                if (gv == NULL)
                {
                    char *_fmt = OSF_Malloc((strlen(expr->v.member_access.child) + 32) * sizeof(char));

                    switch (_red.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_STRING:
                    {
                        sprintf(_fmt, "\'\'.%s", expr->v.member_access.child);
                        gv = IPSF_GetVar_fromMod(mod, _fmt, NULL);
                    }
                    break;
                    case CONSTANT_TYPE_ARRAY:
                    {
                        sprintf(_fmt, "[].%s", expr->v.member_access.child);
                        gv = IPSF_GetVar_fromMod(mod, _fmt, NULL);
                    }
                    break;
                    case CONSTANT_TYPE_DICT:
                    {
                        sprintf(_fmt, "{:}.%s", expr->v.member_access.child);
                        gv = IPSF_GetVar_fromMod(mod, _fmt, NULL);
                    }
                    break;

                    default:
                        break;
                    }

                    OSF_Free(_fmt);
                }

                if (gv == NULL)
                    printf("varname[%s]\n", expr->v.member_access.child);
                assert(gv != NULL && SF_FMT("Error: Object has no member '%s'."));

                _res = gv->val;
                _res.next = OSF_Malloc(sizeof(expr_t));
                *(_res.next) = _red;
                // goto _label_mem_access_fallback;
            }
        }
        break;
        case EXPR_TYPE_MODULE:
        {
            mod_t *gm = PSG_GetModule(_red.v.module_s.index);
            var_t *gv = IPSF_GetVar_fromMod(gm, expr->v.member_access.child, NULL);

            assert(gv != NULL && SF_FMT("Error: Module has no member '%s'."));
            _res = gv->val;
            _res.next = OSF_Malloc(sizeof(expr_t));
            *(_res.next) = _red;
        }
        break;
        default:
        _label_mem_access_fallback:
            // PSG_PrintExprType(_red.type);
            assert(0 && SF_FMT("Cannot overload `.' on %s."));
            break;
        }

        *RES = _res;
    }
    break;
    case EXPR_TYPE_IN_CLAUSE:
    {
        expr_t lhs = IPSF_ReduceExpr_toConstant(mod, *(expr->v.in_clause._lhs)),
               rhs = IPSF_ReduceExpr_toConstant(mod, *(expr->v.in_clause._rhs));

        if (OSF_GetExceptionState())
            goto __label_abrupt_end_IPSF_ExecExprStatement_fromMod;

        *RES = _IPSF_Entity_IsIn_Entity(lhs, rhs, mod);
    }
    break;
    default:
    {
        *RES = *expr;
    }
    break;
    }

__label_abrupt_end_IPSF_ExecExprStatement_fromMod:
    if (OSF_GetExceptionState())
    {
        if (err != NULL)
            *err = IPSF_NOT_OK_CHECK_EXPR_LOG;

        return NULL;
    }
    return RES;
}

struct __mod_child_varhold_s *
IPSF_ExecVarDecl_fromStmt(mod_t *mod, stmt_t stmt, int *err)
{
    struct __mod_child_varhold_s *varhold = NULL;
    if (stmt.type != STATEMENT_TYPE_VAR_DECL)
        exit(printf("Error: seg_fault\n"));

    mod_t *pres = mod->parent;
    expr_t reduced_val_cpy = IPSF_ReduceExpr_toConstant(mod, *(stmt.v.var_decl.expr));

    if (OSF_GetExceptionState())
        return NULL;

    if (mod->type == MODULE_TYPE_CLASS)
        mod->parent = NULL;

    switch (stmt.v.var_decl.name->type)
    {
    case EXPR_TYPE_VARIABLE:
    {
        int _v_idx = -1;

        for (size_t i = 0; i < mod->var_holds_size; i++)
        {
            if (!strcmp(mod->var_holds[i].name, stmt.v.var_decl.name->v.variable.name))
            {
                _v_idx = i;
                break;
            }
        }

        if (_v_idx == -1)
        {
            if (mod->parent != NULL)
            {
                if (IPSF_GetVar_fromMod(mod->parent, stmt.v.var_decl.name->v.variable.name, NULL) != NULL)
                {
                    varhold = IPSF_ExecVarDecl_fromStmt(mod->parent, stmt, err);

                    return varhold;
                }
            }

            if (mod->var_holds_size)
                mod->var_holds = (struct __mod_child_varhold_s *)OSF_Realloc(mod->var_holds,
                                                                             sizeof(struct __mod_child_varhold_s) *
                                                                                 (mod->var_holds_size + 1));

            mod->var_holds[mod->var_holds_size++] = (struct __mod_child_varhold_s){
                .name = stmt.v.var_decl.name->v.variable.name,
                .val = reduced_val_cpy,
            };

            varhold = &(mod->var_holds[mod->var_holds_size - 1]);
        }
        else
        {
            mod->var_holds[_v_idx].val = reduced_val_cpy;
            varhold = &(mod->var_holds[_v_idx]);
        }
    }
    break;
    case EXPR_TYPE_MEMBER_ACCESS:
    {
        expr_t *name_expr = stmt.v.var_decl.name;
        expr_t parent_red = IPSF_ReduceExpr_toConstant(mod, *(name_expr->v.member_access.parent));
        char *symbol_name = name_expr->v.member_access.child;

        switch (parent_red.type)
        {
        case EXPR_TYPE_CONSTANT:
        {
            switch (parent_red.v.constant.constant_type)
            {
            case CONSTANT_TYPE_CLASS_OBJECT:
            {
                class_t *_ref = _IPSF_GetClass_fromIntTuple(parent_red.v.constant.ClassObj.idx);
                mod->parent = pres;

                return IPSF_ExecVarDecl_fromStmt(_ref->mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v = {.var_decl = {.name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v = {.variable = {.name = OSF_strdup(symbol_name)}}, .line = stmt.line}}, .expr = (expr_t *)(expr_t[]){reduced_val_cpy}}}, .line = stmt.line}, NULL);
            }
            break;

            default:
                break;
            }
        }
        break;

        default:
            break;
        }
    }
    break;
    case EXPR_TYPE_INDEX_OP:
    {
        expr_t *c = stmt.v.var_decl.name;
        expr_t ent_red = IPSF_ReduceExpr_toConstant(mod, *(c->v.index_op.entity)),
               idx_red = IPSF_ReduceExpr_toConstant(mod, *(c->v.index_op.index));

        switch (ent_red.type)
        {
        case EXPR_TYPE_CONSTANT:
        {
            switch (ent_red.v.constant.constant_type)
            {
            case CONSTANT_TYPE_ARRAY:
            {
                array_t *_a_ref = PSG_GetArray_Ptr(ent_red.v.constant.Array.index);
                assert(((
                            idx_red.type == EXPR_TYPE_CONSTANT &&
                            idx_red.v.constant.constant_type == CONSTANT_TYPE_INT) ||
                        _IPSF_IsDataType_None(idx_red)) &&
                       SF_FMT("Error: Array index must be an intger"));

                if (_IPSF_IsDataType_None(idx_red))
                {
                    if (_a_ref->len)
                        _a_ref->vals = OSF_Realloc(_a_ref->vals, (_a_ref->len + 1) * sizeof(expr_t));

                    _a_ref->vals[_a_ref->len++] = reduced_val_cpy;
                }
                else
                {
                    int idx = idx_red.v.constant.Int.value;
                    assert(idx < _a_ref->len && SF_FMT("Error: Array index out of range."));

                    if (idx >= _a_ref->len)
                    {
                        OSF_SetException((except_t){
                            .type = EXCEPT_INDEX_OUT_OF_RANGE,
                            .line = stmt.line,
                            .v.ce0 = {
                                .idx = idx,
                                .targ = *_a_ref}});

                        mod->parent = pres;
                        return NULL;
                    }

                    if (idx < 0)
                        idx += _a_ref->len;

                    _a_ref->vals[idx] = reduced_val_cpy;
                }
            }
            break;

            default:
                break;
            }
        }
        break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    if (err != NULL)
        *err = IPSF_OK;

    mod->parent = pres;

    return varhold;
}

struct __mod_child_varhold_s *
IPSF_GetVar_fromMod(mod_t *mod, const char *name, int *err)
{
    struct __mod_child_varhold_s *varhold = NULL;

    if (mod == NULL)
    {
        if (err != NULL)
            *err = IPSF_ERR_VAR_NOT_FOUND;
        return varhold;
    }

    for (size_t i = 0; i < mod->var_holds_size; i++)
    {
        if (!strcmp(mod->var_holds[i].name, name))
        {
            varhold = &(mod->var_holds[i]);
            break;
        }
    }

    struct __mod_child_varhold_s *var_parent_get = IPSF_GetVar_fromMod(mod->parent, name, err);

    if (varhold == NULL && var_parent_get != NULL)
    {
        varhold = var_parent_get;
    }

    if (err != NULL)
    {
        if (varhold == NULL)
            *err = IPSF_ERR_VAR_NOT_FOUND;
        else
            *err = IPSF_OK;
    }

    return varhold;
}

expr_t IPSF_ReduceExpr_toConstant(mod_t *mod, expr_t expr)
{
    expr_t *temp_ex = OSF_Malloc(sizeof(expr_t));
    *temp_ex = expr;
    expr_t *ex_res = IPSF_ExecExprStatement_fromMod(mod, (stmt_t){.type = STATEMENT_TYPE_EXPR, .v.expr.expr = temp_ex, .line = expr.line}, NULL);
    OSF_Free(temp_ex);
    expr_t fex_res;

    if (!OSF_GetExceptionState())
        fex_res = *ex_res;
    else
        fex_res = (expr_t) {
            .type = EXPR_TYPE_CONSTANT,
            .line = expr.line,
            .v.constant = {
                .constant_type = CONSTANT_TYPE_DICT,
                .DType.type = DATA_TYPE_NONE
            }
        };
    
    OSF_Free(ex_res);
    return fex_res;
}

void SF_DestroyModule(mod_t *mod)
{
    if (mod->var_holds && OSF_MemorySafeToFree(mod->var_holds))
        OSF_Free(mod->var_holds);
    if (BODY(mod)->body && OSF_MemorySafeToFree(BODY(mod)->body))
        OSF_Free(BODY(mod)->body);
    if (mod->ast && OSF_MemorySafeToFree(mod->ast))
        OSF_Free(mod->ast);
    if (mod && OSF_MemorySafeToFree(mod))
        OSF_Free(mod);
}

int _IPSF_IsDataType_Void(expr_t expr)
{
    if (expr.type == EXPR_TYPE_CONSTANT)
        if (expr.v.constant.constant_type == CONSTANT_TYPE_DTYPE)
            if (expr.v.constant.DType.type == DATA_TYPE_VOID)
                return 1;

    return 0;
}

int _IPSF_IsDataType_None(expr_t expr)
{
    if (expr.type == EXPR_TYPE_CONSTANT)
        if (expr.v.constant.constant_type == CONSTANT_TYPE_DTYPE)
            if (expr.v.constant.DType.type == DATA_TYPE_NONE)
                return 1;

    return 0;
}

char *_IPSF_ObjectRepr(expr_t expr, int recur)
{
    char *_Res;
    int dy = 0;

    switch (expr.type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        switch (expr.v.constant.constant_type)
        {
        case CONSTANT_TYPE_BOOL:
        {
            _Res = OSF_Malloc(6 * sizeof(char));
            dy = 1;
            sprintf(_Res, expr.v.constant.Bool.value ? "True" : "False");
        }
        break;
        case CONSTANT_TYPE_INT:
        {
            _Res = OSF_Malloc(256 * sizeof(char));
            dy = 1;
            sprintf(_Res, "%d", expr.v.constant.Int.value);
        }
        break;
        case CONSTANT_TYPE_FLOAT:
        {
            _Res = OSF_Malloc(256 * sizeof(char));
            dy = 1;
            sprintf(_Res, "%f", expr.v.constant.Float.value);
        }
        break;
        case CONSTANT_TYPE_STRING:
        {
            _Res = OSF_Malloc((strlen(expr.v.constant.String.value) + 2) * sizeof(char));
            *_Res = '\0';
            dy = 1;
            for (size_t j = recur ? 0 : 1; j < strlen(expr.v.constant.String.value) - (recur ? 0 : 1); j++)
                strcat(_Res, (char[]){expr.v.constant.String.value[j], '\0'});
        }
        break;
        case CONSTANT_TYPE_ARRAY:
        {
            _Res = OSF_Malloc(2 * sizeof(char));
            *_Res = '[';
            *(_Res + 1) = '\0';
            dy = 1;
            for (size_t j = 0; j < ARRAY(expr.v.constant.Array.index).len; j++)
            {
                char *_cop = _IPSF_ObjectRepr(ARRAY(expr.v.constant.Array.index).vals[j], 1);
                _Res = OSF_Realloc(_Res, (strlen(_Res) + strlen(_cop) + 4) * sizeof(char));
                sprintf(_Res, "%s%s", _Res, _cop);
                if (j != (ARRAY(expr.v.constant.Array.index).len) - 1)
                    strcat(_Res, ", ");
            }
            strcat(_Res, "]");
        }
        break;
        case CONSTANT_TYPE_DICT:
        {
            _Res = OSF_Malloc(2 * sizeof(char));
            *_Res = '{';
            *(_Res + 1) = '\0';
            dy = 1;
            for (size_t j = 0; j < DICT(expr.v.constant.Dict.index).len; j++)
            {
                char *_cop = _IPSF_ObjectRepr(DICT(expr.v.constant.Dict.index).keys[j], 1),
                     *_cop_v = _IPSF_ObjectRepr(DICT(expr.v.constant.Dict.index).vals[j], 1);
                _Res = OSF_Realloc(_Res, (strlen(_Res) + strlen(_cop) + strlen(_cop_v) + 8) * sizeof(char));
                sprintf(_Res, "%s%s: %s", _Res, _cop, _cop_v);
                if (j != (DICT(expr.v.constant.Dict.index).len) - 1)
                    strcat(_Res, ", ");
            }
            strcat(_Res, "}");
        }
        break;
        case CONSTANT_TYPE_DTYPE:
        {
            switch (expr.v.constant.DType.type)
            {
            case DATA_TYPE_NONE:
            {
                _Res = OSF_Malloc(5 * sizeof(char));
                dy = 1;
                sprintf(_Res, "None");
            }
            break;
            case DATA_TYPE_VOID:
            {
                _Res = OSF_Malloc(7 * sizeof(char));
                dy = 1;
                sprintf(_Res, "<void>");
            }
            break;

            default:
                break;
            }
        }
        break;
        case CONSTANT_TYPE_CLASS_OBJECT:
        {
            class_t g_c = *_IPSF_GetClass_fromIntTuple(expr.v.constant.ClassObj.idx);

            if (IPSF_GetVar_fromMod(g_c.mod, "__str__", NULL) == NULL)
            {
                if (g_c.name != NULL)
                {
                    _Res = OSF_Malloc((13 + strlen(g_c.name)) * sizeof(char));
                    dy = 1;
                    sprintf(_Res, "<class '%s'>", g_c.name);
                }
                else
                    _Res = "<class '<anonymous>'>";
            }
            else
            {
                expr_t *__r = _IPSF_CallFunction((*PSG_GetFunctions())[IPSF_GetVar_fromMod(g_c.mod, "__str__", NULL)->val.v.function_s.index], (expr_t *)(expr_t[]){expr}, 1, g_c.mod->parent);
                _Res = _IPSF_ObjectRepr(*__r, 0);
                OSF_Free(__r);
            }
        }
        break;
        default:
            _Res = "<seg_fault>";
            break;
        }
    }
    break;
    case EXPR_TYPE_CLASS:
    {
        _Res = OSF_Malloc((13 + strlen((*PSG_GetClasses())[expr.v.class_expr.index].name)) * sizeof(char));
        dy = 1;
        sprintf(_Res, "<class '%s'>", (*PSG_GetClasses())[expr.v.class_expr.index].name);
    }
    break;
    case EXPR_TYPE_FUNCTION:
    {
        if ((*PSG_GetFunctions())[expr.v.function_s.index].name != NULL)
        {
            _Res = OSF_Malloc((16 + strlen((*PSG_GetFunctions())[expr.v.function_s.index].name)) * sizeof(char));
            dy = 1;
            sprintf(_Res, "<function '%s'>", (*PSG_GetFunctions())[expr.v.function_s.index].name);
        }
        else
            _Res = "<function '<anonymous>'>";
    }
    break;
    case EXPR_TYPE_MODULE:
    {
        mod_t *gm = PSG_GetModule(expr.v.module_s.index);

        if (BODY(gm)->name == NULL)
            _Res = "<module '<anonymous>'>";
        else
        {
            _Res = OSF_Malloc((strlen(BODY(gm)->name) + 14) * sizeof(char));
            sprintf(_Res, "<module '%s'>", BODY(gm)->name);
            dy = 1;
        }
    }
    break;
    default:
        _Res = "<seg_fault>";
        break;
    }

    char *rr = OSF_strdup(_Res != NULL ? _Res : "<seg_fault_i342>");

    if (dy)
        OSF_Free(_Res);

    return rr;
}

var_t *_IPSF_AddVar_toModule(mod_t *mod, char *name, expr_t val)
{
    return IPSF_ExecVarDecl_fromStmt(
        mod,
        (stmt_t){
            .type = STATEMENT_TYPE_VAR_DECL,
            .v = {
                .var_decl = {
                    .name = (expr_t *)(expr_t[]){
                        (expr_t){
                            .type = EXPR_TYPE_VARIABLE,
                            .v = {
                                .variable = {
                                    .name = name}}}},
                    .expr = (expr_t *)(expr_t[]){val}}},
            .line = val.line},
        NULL);
}

expr_t _IPSF_ExecUnaryArithmetic(mod_t *mod, expr_t *expr)
{
    expr_t *sibs = OSF_Malloc(expr->v.unary_arith_op_expr.sibs_size * sizeof(expr_t));
    enum ExprArithmeticOpEnum *ops = OSF_Malloc(expr->v.unary_arith_op_expr.ops_size * sizeof(enum ExprArithmeticOpEnum));
    enum ExprArithmeticOpOrderEnum *order = OSF_Malloc(expr->v.unary_arith_op_expr.order_count * sizeof(enum ExprArithmeticOpOrderEnum));

    for (size_t i = 0; i < expr->v.unary_arith_op_expr.sibs_size; i++)
        sibs[i] = IPSF_ReduceExpr_toConstant(mod, expr->v.unary_arith_op_expr.sibs[i]);

    for (size_t i = 0; i < expr->v.unary_arith_op_expr.ops_size; i++)
        ops[i] = expr->v.unary_arith_op_expr.ops[i];

    for (size_t i = 0; i < expr->v.unary_arith_op_expr.order_count; i++)
        order[i] = expr->v.unary_arith_op_expr.order[i];

    expr_t res;
    int constant_count = 0,
        operator_count = 0;

    int sibs_count = expr->v.unary_arith_op_expr.sibs_size;
    int opss_count = expr->v.unary_arith_op_expr.ops_size;
    int order_count = expr->v.unary_arith_op_expr.order_count;

    // Eliminate all '-'
    for (size_t i = 0; i < order_count; i++)
    {
        enum ExprArithmeticOpOrderEnum ci = order[i];

        if (ci == ORDER_OPERATOR)
        {
            enum ExprArithmeticOpEnum *op = &(ops[operator_count]);

            if (*op == UNARY_OP_SUB)
            {
                if (i)
                {
                    int idx_to_modify = constant_count;

                    expr_t *m_ref = &(sibs[idx_to_modify]);

                    assert((
                               m_ref->type == EXPR_TYPE_CONSTANT &&
                               (m_ref->v.constant.constant_type == CONSTANT_TYPE_INT ||
                                m_ref->v.constant.constant_type == CONSTANT_TYPE_FLOAT)) &&
                           SF_FMT("Error: Cannot use unary `-' on a non-number entity."));

                    if (m_ref->v.constant.constant_type == CONSTANT_TYPE_INT)
                        m_ref->v.constant.Int.value = -m_ref->v.constant.Int.value;
                    else
                        m_ref->v.constant.Float.value = -m_ref->v.constant.Float.value;

                    *op = UNARY_OP_ADD;
                }
                else
                {
                    int idx_to_modify = constant_count;

                    expr_t *m_ref = &(sibs[idx_to_modify]);

                    assert((
                               m_ref->type == EXPR_TYPE_CONSTANT &&
                               (m_ref->v.constant.constant_type == CONSTANT_TYPE_INT ||
                                m_ref->v.constant.constant_type == CONSTANT_TYPE_FLOAT)) &&
                           SF_FMT("Error: Cannot use unary `-' on a non-number entity."));

                    if (m_ref->v.constant.constant_type == CONSTANT_TYPE_INT)
                        m_ref->v.constant.Int.value = -m_ref->v.constant.Int.value;
                    else if (m_ref->v.constant.constant_type == CONSTANT_TYPE_FLOAT)
                        m_ref->v.constant.Float.value = -m_ref->v.constant.Float.value;

                    op++;
                    operator_count--;
                    order++;
                    order_count--;
                    i--;
                }
            }

            operator_count++;
        }
        else if (ci == ORDER_CONSTANT)
            constant_count++;
    }

    operator_count = constant_count = 0;

    // '/' operator
    for (size_t i = 0; i < order_count; i++)
    {
        enum ExprArithmeticOpOrderEnum ci = order[i];

        if (ci == ORDER_OPERATOR)
        {
            expr_t lhs = sibs[constant_count - 1],
                   rhs = sibs[constant_count];

            expr_t r;
            r.type = EXPR_TYPE_CONSTANT;

            double RES_LHS = 0,
                   RES_RHS = 0;

            if (ops[operator_count] == UNARY_OP_DIV)
            {
                if (lhs.type == rhs.type)
                {
                    switch (lhs.type)
                    {
                    case EXPR_TYPE_CONSTANT:
                    {
                        if (lhs.v.constant.constant_type == rhs.v.constant.constant_type)
                        {
                            switch (lhs.v.constant.constant_type)
                            {
                            case CONSTANT_TYPE_INT:
                            {
                                RES_LHS = lhs.v.constant.Int.value;
                                RES_RHS = rhs.v.constant.Int.value;
                            }
                            break;
                            case CONSTANT_TYPE_FLOAT:
                            {
                                RES_LHS = lhs.v.constant.Float.value;
                                RES_RHS = rhs.v.constant.Float.value;
                            }
                            break;

                            default:
                                break;
                            }
                        }
                        else
                        {
                            if ((
                                    lhs.v.constant.constant_type == CONSTANT_TYPE_INT ||
                                    lhs.v.constant.constant_type == CONSTANT_TYPE_FLOAT) &&
                                (rhs.v.constant.constant_type == CONSTANT_TYPE_INT ||
                                 rhs.v.constant.constant_type == CONSTANT_TYPE_FLOAT))
                            {
                                RES_LHS = lhs.v.constant.constant_type == CONSTANT_TYPE_INT ? lhs.v.constant.Int.value : lhs.v.constant.Float.value;
                                RES_RHS = rhs.v.constant.constant_type == CONSTANT_TYPE_INT ? rhs.v.constant.Int.value : rhs.v.constant.Float.value;
                            }
                        }
                    }
                    break;

                    default:
                        break;
                    }
                }

                r.v.constant.constant_type = CONSTANT_TYPE_FLOAT;
                r.v.constant.Float.value = (float)RES_LHS / (float)RES_RHS;

                sibs[constant_count - 1] = r;

                expr_t *sibs_cpy = OSF_Malloc(((sibs_count - 1) ? (sibs_count - 1) : 1) * sizeof(expr_t));
                int sibs_sz_cpy = 0;

                for (size_t j = 0; j < sibs_count; j++)
                {
                    if (j == constant_count)
                        continue;
                    sibs_cpy[sibs_sz_cpy++] = sibs[j];
                }

                OSF_Free(sibs);
                sibs = OSF_Malloc(((sibs_count - 1) ? (sibs_count - 1) : 1) * sizeof(expr_t));

                for (size_t j = 0; j < sibs_sz_cpy; j++)
                    sibs[j] = sibs_cpy[j];

                OSF_Free(sibs_cpy);

                enum ExprArithmeticOpEnum *ops_cpy = OSF_Malloc(((opss_count - 1) ? (opss_count - 1) : 1) * sizeof(enum ExprArithmeticOpEnum));
                int ops_c = 0;

                for (size_t j = 0; j < opss_count; j++)
                {
                    if (j == operator_count)
                        continue;

                    ops_cpy[ops_c++] = ops[j];
                }

                OSF_Free(ops);
                ops = OSF_Malloc(((opss_count - 1) ? (opss_count - 1) : 1) * sizeof(enum ExprArithmeticOpEnum));

                for (size_t j = 0; j < ops_c; j++)
                    ops[j] = ops_cpy[j];

                OSF_Free(ops_cpy);

                enum ExprArithmeticOpOrderEnum *order_cpy = OSF_Malloc(((order_count - 2) ? (order_count - 2) : 1) * sizeof(enum ExprArithmeticOpOrderEnum));
                int ocpy_c = 0;

                for (size_t j = 0; j < order_count; j++)
                {
                    if (j >= i && j <= i + 1)
                        continue;
                    order_cpy[ocpy_c++] = order[j];
                }

                OSF_Free(order);
                order = OSF_Malloc(ocpy_c * sizeof(enum ExprArithmeticOpOrderEnum));

                for (size_t j = 0; j < ocpy_c; j++)
                    order[j] = order_cpy[j];

                OSF_Free(order_cpy);

                sibs_count--;
                opss_count--;
                order_count -= 2;

                i = -1; // Start over
                operator_count = constant_count = 0;
                continue;
            }

            operator_count++;
        }
        else if (ci == ORDER_CONSTANT)
            constant_count++;
    }

    // for (size_t i = 0; i < sibs_count; i++)
    // {
    //     if (sibs[i].v.constant.constant_type == CONSTANT_TYPE_INT)
    //         printf("%d\n", sibs[i].v.constant.Int.value);
    //     else
    //         printf("%f\n", sibs[i].v.constant.Float.value);
    // }

    operator_count = constant_count = 0;

    // '*' operator
    for (size_t i = 0; i < order_count; i++)
    {
        enum ExprArithmeticOpOrderEnum ci = order[i];

        if (ci == ORDER_OPERATOR)
        {
            expr_t lhs = sibs[constant_count - 1],
                   rhs = sibs[constant_count];

            expr_t r;
            r.type = EXPR_TYPE_CONSTANT;

            void *RES_LHS = NULL,
                 *RES_RHS = NULL;

            if (ops[operator_count] == UNARY_OP_MUL)
            {
                if (lhs.type == rhs.type)
                {
                    switch (lhs.type)
                    {
                    case EXPR_TYPE_CONSTANT:
                    {
                        if (lhs.v.constant.constant_type == rhs.v.constant.constant_type)
                        {
                            r.v.constant.constant_type = lhs.v.constant.constant_type;
                            switch (lhs.v.constant.constant_type)
                            {
                            case CONSTANT_TYPE_INT:
                            {
                                RES_LHS = (double *)(double[]){lhs.v.constant.Int.value};
                                RES_RHS = (double *)(double[]){rhs.v.constant.Int.value};
                            }
                            break;
                            case CONSTANT_TYPE_FLOAT:
                            {
                                RES_LHS = (double *)(double[]){lhs.v.constant.Float.value};
                                RES_RHS = (double *)(double[]){rhs.v.constant.Float.value};
                            }
                            break;

                            default:
                                break;
                            }
                        }
                        else
                        {
                            if ((
                                    lhs.v.constant.constant_type == CONSTANT_TYPE_INT ||
                                    lhs.v.constant.constant_type == CONSTANT_TYPE_FLOAT) &&
                                (rhs.v.constant.constant_type == CONSTANT_TYPE_INT ||
                                 rhs.v.constant.constant_type == CONSTANT_TYPE_FLOAT))
                            {
                                r.v.constant.constant_type = CONSTANT_TYPE_FLOAT;

                                RES_LHS = (double *)(double[]){lhs.v.constant.constant_type == CONSTANT_TYPE_INT ? lhs.v.constant.Int.value : lhs.v.constant.Float.value};
                                RES_RHS = (double *)(double[]){rhs.v.constant.constant_type == CONSTANT_TYPE_INT ? rhs.v.constant.Int.value : rhs.v.constant.Float.value};
                            }
                            else if (
                                lhs.v.constant.constant_type == CONSTANT_TYPE_STRING ||
                                lhs.v.constant.constant_type == CONSTANT_TYPE_INT)
                            {
                                r.v.constant.constant_type = CONSTANT_TYPE_STRING;

                                RES_LHS = (char *)lhs.v.constant.String.value;
                                RES_RHS = (double *)(double[]){rhs.v.constant.Int.value};
                            }
                        }
                    }
                    break;

                    default:
                        break;
                    }
                }

                switch (r.v.constant.constant_type)
                {
                case CONSTANT_TYPE_INT:
                    r.v.constant.Int.value = (*(double *)RES_LHS) * (*(double *)RES_RHS);
                    break;
                case CONSTANT_TYPE_FLOAT:
                    r.v.constant.Float.value = (*(double *)RES_LHS) * (*(double *)RES_RHS);
                    break;
                case CONSTANT_TYPE_STRING:
                {
                    int ic = *(double *)RES_RHS;
                    if (ic < 1)
                    {
                        r.v.constant.String.value = "\'\'";
                    }
                    else
                    {
                        char *_res_char = OSF_Malloc((strlen((char *)RES_LHS) * ic) * sizeof(char));
                        strcpy(_res_char, "\'");
                        while (ic)
                        {
                            strcat(_res_char, SF_STRING((char *)RES_LHS));
                            ic--;
                        }
                        strcat(_res_char, "\'");
                        char *_r_c_cpy = OSF_strdup(_res_char);
                        OSF_Free(_res_char);
                        r.v.constant.String.value = _r_c_cpy;
                    }
                }
                break;
                default:
                    break;
                }

                sibs[constant_count - 1] = r;

                expr_t *sibs_cpy = OSF_Malloc(((sibs_count - 1) ? (sibs_count - 1) : 1) * sizeof(expr_t));
                int sibs_sz_cpy = 0;

                for (size_t j = 0; j < sibs_count; j++)
                {
                    if (j == constant_count)
                        continue;
                    sibs_cpy[sibs_sz_cpy++] = sibs[j];
                }

                OSF_Free(sibs);
                sibs = OSF_Malloc(((sibs_count - 1) ? (sibs_count - 1) : 1) * sizeof(expr_t));

                for (size_t j = 0; j < sibs_sz_cpy; j++)
                    sibs[j] = sibs_cpy[j];

                OSF_Free(sibs_cpy);

                enum ExprArithmeticOpEnum *ops_cpy = OSF_Malloc(((opss_count - 1) ? (opss_count - 1) : 1) * sizeof(enum ExprArithmeticOpEnum));
                int ops_c = 0;

                for (size_t j = 0; j < opss_count; j++)
                {
                    if (j == operator_count)
                        continue;

                    ops_cpy[ops_c++] = ops[j];
                }

                OSF_Free(ops);
                ops = OSF_Malloc(((opss_count - 1) ? (opss_count - 1) : 1) * sizeof(enum ExprArithmeticOpEnum));

                for (size_t j = 0; j < ops_c; j++)
                    ops[j] = ops_cpy[j];

                OSF_Free(ops_cpy);

                enum ExprArithmeticOpOrderEnum *order_cpy = OSF_Malloc(((order_count - 2) ? (order_count - 2) : 1) * sizeof(enum ExprArithmeticOpOrderEnum));
                int ocpy_c = 0;

                for (size_t j = 0; j < order_count; j++)
                {
                    if (j >= i && j <= i + 1)
                        continue;
                    order_cpy[ocpy_c++] = order[j];
                }

                OSF_Free(order);
                order = OSF_Malloc(ocpy_c * sizeof(enum ExprArithmeticOpOrderEnum));

                for (size_t j = 0; j < ocpy_c; j++)
                    order[j] = order_cpy[j];

                OSF_Free(order_cpy);

                sibs_count--;
                opss_count--;
                order_count -= 2;

                i = -1; // Start over
                operator_count = constant_count = 0;
                continue;
            }

            operator_count++;
        }
        else if (ci == ORDER_CONSTANT)
            constant_count++;
    }

    operator_count = constant_count = 0;

    // '+' operator
    for (size_t i = 0; i < order_count; i++)
    {
        enum ExprArithmeticOpOrderEnum ci = order[i];

        if (ci == ORDER_OPERATOR)
        {
            expr_t lhs = sibs[constant_count - 1],
                   rhs = sibs[constant_count];

            expr_t r;
            r.type = EXPR_TYPE_CONSTANT;
            r.v.constant.constant_type = -1;

            void *RES_LHS = NULL,
                 *RES_RHS = NULL;

            if (ops[operator_count] == UNARY_OP_ADD)
            {
                if (lhs.type == rhs.type)
                {
                    switch (lhs.type)
                    {
                    case EXPR_TYPE_CONSTANT:
                    {
                        if (lhs.v.constant.constant_type == rhs.v.constant.constant_type)
                        {
                            r.v.constant.constant_type = lhs.v.constant.constant_type;
                            switch (lhs.v.constant.constant_type)
                            {
                            case CONSTANT_TYPE_INT:
                            {
                                RES_LHS = (double *)(double[]){lhs.v.constant.Int.value};
                                RES_RHS = (double *)(double[]){rhs.v.constant.Int.value};
                            }
                            break;
                            case CONSTANT_TYPE_FLOAT:
                            {
                                RES_LHS = (double *)(double[]){lhs.v.constant.Float.value};
                                RES_RHS = (double *)(double[]){rhs.v.constant.Float.value};
                            }
                            break;
                            case CONSTANT_TYPE_STRING:
                            {
                                RES_LHS = (char *)lhs.v.constant.String.value;
                                RES_RHS = (char *)rhs.v.constant.String.value;
                            }
                            break;
                            default:
                                break;
                            }
                        }
                        else
                        {
                            if ((
                                    lhs.v.constant.constant_type == CONSTANT_TYPE_INT ||
                                    lhs.v.constant.constant_type == CONSTANT_TYPE_FLOAT) &&
                                (rhs.v.constant.constant_type == CONSTANT_TYPE_INT ||
                                 rhs.v.constant.constant_type == CONSTANT_TYPE_FLOAT))
                            {
                                r.v.constant.constant_type = CONSTANT_TYPE_FLOAT;

                                RES_LHS = (double *)(double[]){lhs.v.constant.constant_type == CONSTANT_TYPE_INT ? lhs.v.constant.Int.value : lhs.v.constant.Float.value};
                                RES_RHS = (double *)(double[]){rhs.v.constant.constant_type == CONSTANT_TYPE_INT ? rhs.v.constant.Int.value : rhs.v.constant.Float.value};
                            }
                        }
                    }
                    break;

                    default:
                        break;
                    }
                }

                switch (r.v.constant.constant_type)
                {
                case CONSTANT_TYPE_INT:
                    r.v.constant.Int.value = *(double *)RES_LHS + *(double *)RES_RHS;
                    break;
                case CONSTANT_TYPE_FLOAT:
                    r.v.constant.Float.value = *(double *)RES_LHS + *(double *)RES_RHS;
                    break;
                case CONSTANT_TYPE_STRING:
                {
                    r.v.constant.String.value = OSF_Malloc((strlen((char *)RES_LHS) + strlen((char *)RES_RHS) + 1) * sizeof(char));
                    RES_RHS++;
                    ((char *)RES_LHS)[strlen(((char *)RES_LHS)) - 1] = '\0';
                    sprintf(r.v.constant.String.value, "%s%s", RES_LHS, RES_RHS);
                }
                break;
                case -1:
                default:
                    assert(0 && SF_FMT("Error: Invalid operator usage: `+'."));
                    break;
                }

                sibs[constant_count - 1] = r;

                expr_t *sibs_cpy = OSF_Malloc(((sibs_count - 1) ? (sibs_count - 1) : 1) * sizeof(expr_t));
                int sibs_sz_cpy = 0;

                for (size_t j = 0; j < sibs_count; j++)
                {
                    if (j == constant_count)
                        continue;
                    sibs_cpy[sibs_sz_cpy++] = sibs[j];
                }

                OSF_Free(sibs);
                sibs = OSF_Malloc(((sibs_count - 1) ? (sibs_count - 1) : 1) * sizeof(expr_t));

                for (size_t j = 0; j < sibs_sz_cpy; j++)
                    sibs[j] = sibs_cpy[j];

                OSF_Free(sibs_cpy);

                enum ExprArithmeticOpEnum *ops_cpy = OSF_Malloc(((opss_count - 1) ? (opss_count - 1) : 1) * sizeof(enum ExprArithmeticOpEnum));
                int ops_c = 0;

                for (size_t j = 0; j < opss_count; j++)
                {
                    if (j == operator_count)
                        continue;

                    ops_cpy[ops_c++] = ops[j];
                }

                OSF_Free(ops);
                ops = OSF_Malloc(((opss_count - 1) ? (opss_count - 1) : 1) * sizeof(enum ExprArithmeticOpEnum));

                for (size_t j = 0; j < ops_c; j++)
                    ops[j] = ops_cpy[j];

                OSF_Free(ops_cpy);

                enum ExprArithmeticOpOrderEnum *order_cpy = OSF_Malloc(((order_count - 2) ? (order_count - 2) : 1) * sizeof(enum ExprArithmeticOpOrderEnum));
                int ocpy_c = 0;

                for (size_t j = 0; j < order_count; j++)
                {
                    if (j >= i && j <= i + 1)
                        continue;
                    order_cpy[ocpy_c++] = order[j];
                }

                OSF_Free(order);
                order = OSF_Malloc(ocpy_c * sizeof(enum ExprArithmeticOpOrderEnum));

                for (size_t j = 0; j < ocpy_c; j++)
                    order[j] = order_cpy[j];

                OSF_Free(order_cpy);

                sibs_count--;
                opss_count--;
                order_count -= 2;

                i = -1; // Start over
                operator_count = constant_count = 0;
                continue;
            }

            operator_count++;
        }
        else if (ci == ORDER_CONSTANT)
            constant_count++;
    }

    operator_count = constant_count = 0;

    res = *sibs;

    OSF_Free(sibs);
    OSF_Free(ops);
    OSF_Free(order);

    return res;
}

var_t *_IPSF_AddVar_toModule_strict(mod_t *mod, char *name, expr_t val)
{
    int var_idx = -1;

    for (size_t i = 0; i < mod->var_holds_size; i++)
    {
        if (!strcmp(mod->var_holds[i].name, name))
        {
            var_idx = i;
            break;
        }
    }

    if (var_idx == -1)
    {
        if (mod->var_holds_size)
            mod->var_holds = OSF_Realloc(mod->var_holds, (mod->var_holds_size + 1) * sizeof(var_t));

        mod->var_holds[mod->var_holds_size++] = (var_t){
            .name = name,
            .val = val};

        var_idx = mod->var_holds_size - 1;
    }
    else
        mod->var_holds[var_idx].val = val;

    return &(mod->var_holds[var_idx]);
}

stmt_t *_SFPTR_StmtPtr(stmt_t stmt)
{
    stmt_t *s = OSF_Malloc(sizeof(stmt_t));
    *s = stmt;
    return s;
}

expr_t *_IPSF_CallFunction(fun_t fun_s, expr_t *args, int arg_size, mod_t *mod)
{
    expr_t *RES = OSF_Malloc(sizeof(expr_t));

    if (fun_s.arg_acceptance_count >= 0)
        assert(arg_size == fun_s.arg_acceptance_count && SF_FMT("Error: Invalid number of arguments passed."));

    mod_t *fun_mod = SF_CreateModule(MODULE_TYPE_FUNCTION, NULL);
    OSF_Free(BODY(fun_mod)->body);

    expr_t *_collectivise_args = OSF_Malloc(sizeof(expr_t) * (arg_size ? arg_size : 1));
    int _collectivise_args_count = 0;
    expr_t *_cargs_without_voids;
    int _cargs_without_voids_size = 0;

    if (fun_s.is_native)
    {
        for (size_t j = 0; j < fun_s.v.Native.arg_size; j++)
        {
            expr_t *texpr = OSF_Malloc(sizeof(expr_t));

            if (fun_s.v.Native.args[j].next == NULL)
                *texpr = (expr_t){
                    .type = EXPR_TYPE_CONSTANT,
                    .v.constant = {
                        .constant_type = CONSTANT_TYPE_DTYPE,
                        .DType = {
                            .type = DATA_TYPE_VOID}},
                    .line = args[j].line};
            else
                *texpr = *(fun_s.v.Native.args[j].next);

            IPSF_ExecVarDecl_fromStmt(
                fun_mod,
                (stmt_t){
                    .type = STATEMENT_TYPE_VAR_DECL,
                    .v = {
                        .var_decl = {
                            .name = (expr_t *)(expr_t[]){
                                (expr_t){
                                    .type = EXPR_TYPE_VARIABLE,
                                    .v = {
                                        .variable = {
                                            .name = fun_s.v.Native.args[j].v.variable.name}},
                                    .line = args[j].line}},
                            .expr = texpr}},
                    .line = args[j].line},
                NULL);

            OSF_Free(texpr);
        }
    }
    else
    {
        for (size_t j = 0; j < fun_s.v.Coded.arg_size; j++)
        {
            _IPSF_AddVar_toModule(fun_mod, fun_s.v.Coded.args[j].name, fun_s.v.Coded.args[j].val);
        }
    }

    for (size_t j = 0; j < arg_size; j++)
    {
        if (args[j].type == EXPR_TYPE_INLINE_ASSIGNMENT)
            _collectivise_args[_collectivise_args_count++] = IPSF_ReduceExpr_toConstant(fun_mod, args[j]);
        else
            _collectivise_args[_collectivise_args_count++] = args[j];
    }

    if (OSF_GetExceptionState())
        return NULL;

    fun_mod->parent = fun_s.parent;
    _cargs_without_voids = _PSF_RemoveVoidsFromExprArray(_collectivise_args, _collectivise_args_count, &_cargs_without_voids_size);

    switch (fun_s.arg_acceptance_count)
    {
    case -1:
    {
        var_t *first_void_ref = NULL;

        for (size_t j = 0; j < fun_mod->var_holds_size; j++)
        {
            if (_IPSF_IsDataType_Void(fun_mod->var_holds[j].val))
            {
                first_void_ref = &(fun_mod->var_holds[j]);
                break;
            }
        }

        assert(first_void_ref != NULL && SF_FMT("Error: Function with variable arguments needs to have an undefined variable"));

        (*first_void_ref).val = (expr_t){
            .type = EXPR_TYPE_CONSTANT,
            .v = {
                .constant = {
                    .constant_type = CONSTANT_TYPE_ARRAY,
                    .Array = {
                        .index = PSG_AddArray(Sf_Array_New_fromExpr(_cargs_without_voids, _cargs_without_voids_size))}}},
            .line = BODY(fun_mod)->body->line};

        first_void_ref = NULL;
    }
    break;
    case -2:
    {
        int _t = 0;
        for (size_t j = 0; j < _collectivise_args_count; j++)
        {
            if (_IPSF_IsDataType_Void(_collectivise_args[j]))
                continue;

            fun_mod->var_holds[_t++].val = _collectivise_args[j];
        }
    }
    break;

    default:
    {
        int _t = 0;
        for (size_t j = 0; j < fun_mod->var_holds_size; j++)
        {
            if (_IPSF_IsDataType_Void(_collectivise_args[j]))
                continue;

            while (!_IPSF_IsDataType_Void(fun_mod->var_holds[_t].val) && _t < fun_mod->var_holds_size)
                _t++;

            if (_t < fun_mod->var_holds_size)
                fun_mod->var_holds[_t++].val = _collectivise_args[j];
        }
    }
    break;
    }

    if (fun_s.parent != NULL)
    {
        for (size_t i = 0; i < fun_s.parent->var_holds_size; i++)
        {
            _IPSF_AddVar_toModule(
                fun_mod,
                fun_s.parent->var_holds[i].name,
                fun_s.parent->var_holds[i].val);
        }
    }

    if (fun_s.is_native)
    {
        expr_t *res = fun_s.v.Native.f(fun_mod);
        expr_t temp = *res;
        *RES = temp;
        OSF_Free(res);
    }
    else
    {
        BODY(fun_mod)->body = fun_s.v.Coded.body;
        BODY(fun_mod)->body_size = fun_s.v.Coded.body_size;
        BODY(fun_mod)->name = NULL;
        OSF_Free(IPSF_ExecIT_fromMod(fun_mod, NULL));

        if (fun_mod->returns == NULL)
        {
            *RES = (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v = {
                    .constant = {
                        .constant_type = CONSTANT_TYPE_DTYPE,
                        .DType = {
                            .type = DATA_TYPE_NONE}}},
                .line = BODY(fun_mod)->body->line};
        }
        else
        {
            expr_t *res = fun_mod->returns;
            expr_t temp = *res;
            *RES = temp;
            OSF_Free(res);
        }
    }

    OSF_Free(_cargs_without_voids);
    OSF_Free(_collectivise_args);

    // SF_Module_safeDelete(fun_mod);
    // OSF_Free(fun_mod);

    return RES;
}

void SF_Module_safeDelete(mod_t *mod)
{
    // if (OSF_MemorySafeToFree(mod->var_holds))
    OSF_Free(mod->var_holds);

    mod->var_holds_size = 0;
}

void _IPSF_DestClasses(mod_t *mod)
{
    for (size_t i = 0; i < mod->var_holds_size; i++)
    {
        var_t curr = mod->var_holds[i];

        if (curr.val.type == EXPR_TYPE_CONSTANT)
        {
            if (curr.val.v.constant.constant_type == CONSTANT_TYPE_CLASS_OBJECT)
            {
                class_t *c = _IPSF_GetClass_fromIntTuple(curr.val.v.constant.ClassObj.idx);

                if (c->killed)
                    continue;

                c->killed = 1;

                int fv = IPSF_OK;
                var_t *get_kill = IPSF_GetVar_fromMod(c->mod, "__kill__", &fv);

                if (fv == IPSF_ERR_VAR_NOT_FOUND)
                    continue;

                expr_t k_ev = get_kill->val;
                if (k_ev.type == EXPR_TYPE_FUNCTION)
                {
                    fun_t _k_ev_f = (*PSG_GetFunctions())[k_ev.v.function_s.index];
                    expr_t *arg_arr = OSF_Malloc(sizeof(expr_t));
                    *arg_arr = (expr_t){
                        .type = EXPR_TYPE_CONSTANT,
                        .v = {
                            .constant = {
                                .constant_type = CONSTANT_TYPE_CLASS_OBJECT,
                                .ClassObj = {
                                    .idx = curr.val.v.constant.ClassObj.idx}}},
                        .line = 0};

                    OSF_Free(_IPSF_CallFunction(_k_ev_f, arg_arr, 1, mod));
                    OSF_Free(arg_arr);
                }
            }
        }
    }
}

class_t *_IPSF_GetClass_fromIntTuple(int_tuple tup)
{
    return &((*PSG_GetClasses())[tup.x1].objects[tup.x2]);
}

expr_t _IPSF_ExecLogicalArithmetic(expr_t _lhs, int op_ty, expr_t _rhs)
{
    expr_t _res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_BOOL}},
        .line = _lhs.line};

    switch (op_ty)
    {
    case LOGICAL_OP_EQEQ:
    {
        if (_lhs.type == _rhs.type)
        {
            switch (_lhs.type)
            {
            case EXPR_TYPE_CONSTANT:
            {
                if (_lhs.v.constant.constant_type ==
                    _rhs.v.constant.constant_type)
                {
                    switch (_lhs.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_BOOL:
                        _res.v.constant.Bool.value = _lhs.v.constant.Bool.value == _rhs.v.constant.Bool.value;
                        break;
                    case CONSTANT_TYPE_DTYPE:
                        _res.v.constant.Bool.value = _lhs.v.constant.DType.type == _rhs.v.constant.DType.type;
                        break;
                    case CONSTANT_TYPE_FLOAT:
                    {
                        if (_lhs.v.constant.Float.is_inf &&
                            _rhs.v.constant.Float.is_inf)
                            _res.v.constant.Bool.value = 1;
                        else
                            _res.v.constant.Bool.value = _lhs.v.constant.Float.value == _rhs.v.constant.Float.value;
                    }
                    break;
                    case CONSTANT_TYPE_INT:
                        _res.v.constant.Bool.value = _lhs.v.constant.Int.value == _rhs.v.constant.Int.value;
                        break;
                    case CONSTANT_TYPE_STRING:
                        _res.v.constant.Bool.value = !strcmp(_lhs.v.constant.String.value, _rhs.v.constant.String.value);
                        break;
                    default:
                        assert(0 && SF_FMT("Unknown entities to compare"));
                        _res.v.constant.Bool.value = 0;
                        break;
                    }
                }
                else
                {
                    _res.v.constant.Bool.value = 0;
                }
            }
            break;

            default:
                break;
            }
        }
        else
        {
            _res.v.constant.Bool.value = 0;
        }
    }
    break;
    case LOGICAL_OP_NEQ:
    {
        if (_lhs.type == _rhs.type)
        {
            switch (_lhs.type)
            {
            case EXPR_TYPE_CONSTANT:
            {
                if (_lhs.v.constant.constant_type ==
                    _rhs.v.constant.constant_type)
                {
                    switch (_lhs.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_BOOL:
                        _res.v.constant.Bool.value = _lhs.v.constant.Bool.value != _rhs.v.constant.Bool.value;
                        break;
                    case CONSTANT_TYPE_DTYPE:
                        _res.v.constant.Bool.value = _lhs.v.constant.DType.type != _rhs.v.constant.DType.type;
                        break;
                    case CONSTANT_TYPE_FLOAT:
                    {
                        if (_lhs.v.constant.Float.is_inf &&
                            _rhs.v.constant.Float.is_inf)
                            _res.v.constant.Bool.value = 0;
                        else
                            _res.v.constant.Bool.value = _lhs.v.constant.Float.value != _rhs.v.constant.Float.value;
                    }
                    break;
                    case CONSTANT_TYPE_INT:
                        _res.v.constant.Bool.value = _lhs.v.constant.Int.value != _rhs.v.constant.Int.value;
                        break;
                    case CONSTANT_TYPE_STRING:
                        _res.v.constant.Bool.value = !!strcmp(_lhs.v.constant.String.value, _rhs.v.constant.String.value);
                        break;
                    default:
                        assert(0 && SF_FMT("Unknown entities to compare"));
                        _res.v.constant.Bool.value = 1;
                        break;
                    }
                }
                else
                {
                    _res.v.constant.Bool.value = 1;
                }
            }
            break;

            default:
                break;
            }
        }
        else
        {
            _res.v.constant.Bool.value = 1;
        }
    }
    break;
    case LOGICAL_OP_GEQ:
    {
        if (_lhs.type == _rhs.type)
        {
            switch (_lhs.type)
            {
            case EXPR_TYPE_CONSTANT:
            {
                if (_lhs.v.constant.constant_type ==
                    _rhs.v.constant.constant_type)
                {
                    switch (_lhs.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_FLOAT:
                    {
                        if (_lhs.v.constant.Float.is_inf &&
                            _rhs.v.constant.Float.is_inf)
                            _res.v.constant.Bool.value = 0;
                        else
                            _res.v.constant.Bool.value = _lhs.v.constant.Float.value > _rhs.v.constant.Float.value;
                    }
                    break;
                    case CONSTANT_TYPE_INT:
                        _res.v.constant.Bool.value = _lhs.v.constant.Int.value > _rhs.v.constant.Int.value;
                        break;
                    case CONSTANT_TYPE_STRING:
                        _res.v.constant.Bool.value = strcmp(_lhs.v.constant.String.value, _rhs.v.constant.String.value) > 0;
                        break;
                    default:
                        assert(0 && SF_FMT("Unknown entities to compare"));
                        _res.v.constant.Bool.value = 1;
                        break;
                    }
                }
                else
                {
                    // TODO: here
                    _res.v.constant.Bool.value = 1;
                }
            }
            break;

            default:
                break;
            }
        }
        else
        {
            // TODO: here
            _res.v.constant.Bool.value = 1;
        }
    }
    break;
    case LOGICAL_OP_LEQ:
    {
        if (_lhs.type == _rhs.type)
        {
            switch (_lhs.type)
            {
            case EXPR_TYPE_CONSTANT:
            {
                if (_lhs.v.constant.constant_type ==
                    _rhs.v.constant.constant_type)
                {
                    switch (_lhs.v.constant.constant_type)
                    {
                    case CONSTANT_TYPE_FLOAT:
                    {
                        if (_lhs.v.constant.Float.is_inf &&
                            _rhs.v.constant.Float.is_inf)
                            _res.v.constant.Bool.value = 0;
                        else
                            _res.v.constant.Bool.value = _lhs.v.constant.Float.value < _rhs.v.constant.Float.value;
                    }
                    break;
                    case CONSTANT_TYPE_INT:
                        _res.v.constant.Bool.value = _lhs.v.constant.Int.value < _rhs.v.constant.Int.value;
                        break;
                    case CONSTANT_TYPE_STRING:
                        _res.v.constant.Bool.value = strcmp(_lhs.v.constant.String.value, _rhs.v.constant.String.value) < 0;
                        break;
                    default:
                        assert(0 && SF_FMT("Unknown entities to compare"));
                        _res.v.constant.Bool.value = 1;
                        break;
                    }
                }
                else
                {
                    // TODO: here
                    _res.v.constant.Bool.value = 1;
                }
            }
            break;

            default:
                break;
            }
        }
        else
        {
            // TODO: here
            _res.v.constant.Bool.value = 1;
        }
    }
    break;
    case LOGICAL_OP_GEEQ:
    {
        _res = _IPSF_ExecLogicalArithmetic(_lhs, LOGICAL_OP_LEQ, _rhs);
        _res.v.constant.Bool.value = abs(_res.v.constant.Bool.value - 1); // 0 becomes 1, 1 becomes 0
    }
    break;
    case LOGICAL_OP_LEEQ:
    {
        _res = _IPSF_ExecLogicalArithmetic(_lhs, LOGICAL_OP_GEQ, _rhs);
        _res.v.constant.Bool.value = abs(_res.v.constant.Bool.value - 1); // 0 becomes 1, 1 becomes 0
    }
    break;

    default:
        break;
    }

    return _res;
}

expr_t _IPSF_Entity_IsIn_Entity(expr_t lhs, expr_t rhs, mod_t *mod)
{
    expr_t res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_BOOL,
                .Bool = {
                    .value = 0}}},
        .line = lhs.line};

    switch (rhs.type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        switch (rhs.v.constant.constant_type)
        {
        case CONSTANT_TYPE_ARRAY:
        {
            array_t arr = ARRAY(rhs.v.constant.Array.index);

            for (size_t i = 0; i < arr.len; i++)
            {
                if (_PSF_EntityIsTrue(_IPSF_ExecLogicalArithmetic(lhs, LOGICAL_OP_EQEQ, arr.vals[i])))
                {
                    res.v.constant.Bool.value = 1;
                    break;
                }
            }
        }
        break;
        case CONSTANT_TYPE_STRING:
        {
            assert(
                (lhs.type == EXPR_TYPE_CONSTANT &&
                 lhs.v.constant.constant_type == CONSTANT_TYPE_STRING) &&
                SF_FMT("Error: Entity must be string."));

            char *lhs_s = SF_STRING(lhs.v.constant.String.value),
                 *rhs_s = SF_STRING(rhs.v.constant.String.value);

            res.v.constant.Bool.value = (char *)strstr(rhs_s, lhs_s) != NULL;
        }
        break;

        default:
            goto _entity_in_entity_fallback_label;
            break;
        }
    }
    break;

    default:
    _entity_in_entity_fallback_label:
        assert(0 && SF_FMT("Error: Invalid iterator."));
        break;
    }

    return res;
}

char *_IPSF_GetDir_FromFilePath(char *path)
{
    char *pcpy = OSF_strdup(path);
    char *new_path = strrev(pcpy);

    while (1)
        if (*new_path == '/' ||
            *new_path == '\\')
            break;
        else
            new_path++;

    return strrev(new_path);
}
