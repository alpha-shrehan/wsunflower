#include "built_in.h"
#include <Parser/array_t.h>

expr_t *NativeFunctionWrite(mod_t *mod_ref)
{
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v.constant = {
            .constant_type = CONSTANT_TYPE_DTYPE,
            .DType = {
                .type = DATA_TYPE_NONE}}};
    
    var_t *args_v = IPSF_GetVar_fromMod(mod_ref, "args", NULL);
    assert((args_v->val.type == EXPR_TYPE_CONSTANT && args_v->val.v.constant.constant_type == CONSTANT_TYPE_ARRAY) && SF_FMT("`args' must be an array"));
    expr_t *args = ARRAY(args_v->val.v.constant.Array.index).vals;
    int arg_count = ARRAY(args_v->val.v.constant.Array.index).len;

    var_t *var_end = IPSF_GetVar_fromMod(mod_ref, "end", NULL),
          *var_join = IPSF_GetVar_fromMod(mod_ref, "join", NULL);

    for (size_t i = 0; i < arg_count; i++)
    {
        int print_done = 0;
        if (args[i].type == EXPR_TYPE_CONSTANT)
        {
            if (args[i].v.constant.constant_type == CONSTANT_TYPE_CLASS_OBJECT)
            {
                /**
                 * @brief Check for __stdout__(self)
                 * If found, use it
                 * If not found, check for __str__(self)
                 * If found, use it
                 * Else, do standard printing
                 */
                class_t *cobj = _IPSF_GetClass_fromIntTuple(args[i].v.constant.ClassObj.idx);
                int fe = IPSF_OK;
                var_t *g__stdout__ = IPSF_GetVar_fromMod(cobj->mod, "__stdout__", &fe);
                fun_t _fun_exec;

                if (fe != IPSF_ERR_VAR_NOT_FOUND)
                {
                    if (g__stdout__->val.type != EXPR_TYPE_FUNCTION)
                        goto _native_label_fun_write__str__check_label;

                    _fun_exec = (*PSG_GetFunctions())[g__stdout__->val.v.function_s.index];
                    print_done = 1;
                }
                else
                {
                _native_label_fun_write__str__check_label:
                    fe = IPSF_OK;
                    var_t *g__str__ = IPSF_GetVar_fromMod(cobj->mod, "__str__", &fe);

                    if (fe != IPSF_ERR_VAR_NOT_FOUND)
                    {
                        if (g__str__->val.type == EXPR_TYPE_FUNCTION)
                        {
                            _fun_exec = (*PSG_GetFunctions())[g__str__->val.v.function_s.index];
                            print_done = 1;
                        }
                    }
                }

                if (print_done) // Not really done, just a check to see if we shall do it
                {
                    expr_t *_v_res = _IPSF_CallFunction(_fun_exec, &(args[i]), 1, mod_ref);

                    if (OSF_GetExceptionState())
                        return __res;

                    char *x = _IPSF_ObjectRepr(*_v_res, 0);

                    if (OSF_GetExceptionState())
                        return __res;
                    
                    printf("%s", x);

                    OSF_IntFree(x);
                    OSF_Free(_v_res);
                }
            }
        }

        char *x = _IPSF_ObjectRepr(args[i], 0);

        if (!print_done)
            printf("%s", x);

        if (i != arg_count - 1)
        {
            if (var_join != NULL)
            {
                char *x = _IPSF_ObjectRepr(var_join->val, 0);
                printf("%s", x);

                OSF_IntFree(x);
            }
            else
            {
                printf(" ");
            }
        }
        OSF_IntFree(x);
    }

    if (var_end != NULL)
    {
        char *x = _IPSF_ObjectRepr(var_end->val, 0);
        printf("%s", x);
        OSF_IntFree(x);
    }

    return __res;
}

expr_t *NativeFunctionInput(mod_t *mod_ref)
{
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    expr_t args[1];
    int arg_count = 1;

    var_t *arg_v = IPSF_GetVar_fromMod(mod_ref, "msg", NULL);
    *args = arg_v->val;

    for (size_t i = 0; i < arg_count; i++)
    {
        // printf("%d\n", *(int *)SF_CCast_Entity(args[i]));
        switch (args[i].type)
        {
        case EXPR_TYPE_CONSTANT:
        {
            switch (args[i].v.constant.constant_type)
            {
            case CONSTANT_TYPE_BOOL:
                printf("%s", args[i].v.constant.Bool.value ? "True" : "False");
                break;
            case CONSTANT_TYPE_INT:
                printf("%d", args[i].v.constant.Int.value);
                break;
            case CONSTANT_TYPE_FLOAT:
                printf("%f", args[i].v.constant.Float.value);
                break;
            case CONSTANT_TYPE_STRING:
                for (size_t j = 1; j < strlen(args[i].v.constant.String.value) - 1; j++)
                    printf("%c", args[i].v.constant.String.value[j]);
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

    char *_fdata = OSF_Malloc(sizeof(char));
    int _fcount = 1;
    char c;

    *_fdata = '\'';

    while ((c = fgetc(stdin)) != '\n')
    {
        _fdata = OSF_Realloc(_fdata, (_fcount + 1) * sizeof(char));
        _fdata[_fcount++] = c;
    }

    _fdata = OSF_Realloc(_fdata, (_fcount + 2) * sizeof(char));
    _fdata[_fcount++] = '\'';
    _fdata[_fcount++] = '\0';

    const char *_f_dup = OSF_strdup(_fdata);
    OSF_Free(_fdata);
    _fdata = NULL;

    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v.constant = {
            .constant_type = CONSTANT_TYPE_STRING,
            .String.value = (char *)_f_dup}};

    return __res;
}

expr_t *NativeFunctionInt(mod_t *_mod_ref)
{
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    expr_t *args = OSF_Malloc(sizeof(expr_t));
    int arg_count = 1;

    var_t *ent_a = IPSF_GetVar_fromMod(_mod_ref, "num", NULL);
    *args = ent_a->val;

    assert((
               args[0].type == EXPR_TYPE_CONSTANT &&
               args[0].v.constant.constant_type == CONSTANT_TYPE_STRING) &&
           SF_FMT("`num' must be a string"));
        
    if (OSF_GetExceptionState())
        return NULL;

    char *get_str = args[0].v.constant.String.value;
    get_str++;
    get_str[strlen(get_str) - 1] = '\0';
    int i_res = atoi(get_str);

    OSF_Free(args);

    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_INT,
                .Int = {
                    .value = i_res}}}};

    return __res;
}

expr_t *NativeFunctionEval(mod_t *mod_ref)
{
    expr_t *__res; // reference acquired later
    var_t *arg_icode = IPSF_GetVar_fromMod(mod_ref, "_Code", NULL);
    assert(
        (
            arg_icode->val.type == EXPR_TYPE_CONSTANT &&
            arg_icode->val.v.constant.constant_type == CONSTANT_TYPE_STRING) &&
        SF_FMT("eval(...) expects a string, got %s"));

    expr_t ival = arg_icode->val;
    char *ival_str = ival.v.constant.String.value;
    ival_str++;
    ival_str[strlen(ival_str) - 1] = '\0';

    psf_byte_array_t *ast;
    ast = PSF_AST_fromString(ival_str);
    PSF_AST_Preprocess_fromByteArray(ast);

    mod_t *eval_mod = SF_CreateModule(MODULE_TYPE_FILE, ast);
    SF_FrameIT_fromAST(eval_mod);

    var_t *mv_hold = mod_ref->var_holds;
    mod_ref->var_holds = OSF_Malloc(sizeof(var_t));
    int mvs_hold = mod_ref->var_holds_size;
    mod_ref->var_holds_size = 0;

    eval_mod->parent = mod_ref;

    int err;
    __res = IPSF_ExecIT_fromMod(eval_mod, &err);

    assert(err == IPSF_OK && SF_FMT(""));

    OSF_Free(mod_ref->var_holds);
    mod_ref->var_holds = mv_hold;
    mod_ref->var_holds_size = mvs_hold;

    return __res;
}

expr_t *NativeFunctionnativeType(mod_t *mod_ref)
{
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    var_t *get_v = IPSF_GetVar_fromMod(mod_ref, "_Val", NULL);
    expr_t v = get_v->val;

    __res->type = EXPR_TYPE_CONSTANT;
    __res->v.constant.constant_type = CONSTANT_TYPE_STRING;

    switch (v.type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        switch (v.v.constant.constant_type)
        {
        case CONSTANT_TYPE_INT:
            __res->v.constant.String.value = "\'def:int\'";
            break;
        case CONSTANT_TYPE_FLOAT:
            __res->v.constant.String.value = "\'def:float\'";
            break;
        case CONSTANT_TYPE_BOOL:
            __res->v.constant.String.value = "\'def:bool\'";
            break;
        case CONSTANT_TYPE_STRING:
            __res->v.constant.String.value = "\'def:str\'";
            break;
        case CONSTANT_TYPE_DTYPE:
            __res->v.constant.String.value = "\'def:dtype\'";
            break;
        default:
            __res->v.constant.String.value = "\'\'";
            break;
        }
    }
    break;

    default:
        break;
    }

    return __res;
}

expr_t *NativeFunctionLen(mod_t *mod_ref)
{
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    var_t *g_v = IPSF_GetVar_fromMod(mod_ref, "_Val", NULL);
    expr_t gve = g_v->val;
    __res->type = EXPR_TYPE_CONSTANT;
    __res->v.constant.constant_type = CONSTANT_TYPE_INT;

    switch (gve.type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        switch (gve.v.constant.constant_type)
        {
        case CONSTANT_TYPE_ARRAY:
            __res->v.constant.Int.value = ARRAY(gve.v.constant.Array.index).len;
            break;
        case CONSTANT_TYPE_STRING:
            __res->v.constant.Int.value = strlen(gve.v.constant.String.value) - 2;
            break;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    return __res;
}

void SFBuiltIn_AddDefaultFunctions(mod_t *mod)
{
    fun_t _write_fun = (fun_t){
        .name = "write",
        .is_native = 1,
        .arg_acceptance_count = -1,
        .v.Native = {
            .arg_size = 3,
            .args = OSF_Malloc(3 * sizeof(expr_t)),
            .f = NativeFunctionWrite},
        .parent = mod};

    _write_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "args"},
        .next = NULL};

    _write_fun.v.Native.args[1] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .next = OSF_Malloc(sizeof(expr_t)),
        .v.variable = {.name = "end"}};

    *(_write_fun.v.Native.args[1].next) = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_STRING,
                .String = {
                    .value = "\'\n\'"}}}};

    _write_fun.v.Native.args[2] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .next = OSF_Malloc(sizeof(expr_t)),
        .v.variable = {.name = "join"}};

    *(_write_fun.v.Native.args[2].next) = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_STRING,
                .String = {
                    .value = "\' \'"}}}};

    int fun_write_mem_idx = PSG_AddFunction(_write_fun);

    fun_t _input_fun = (fun_t){
        .name = "input",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionInput},
        .parent = mod};

    _input_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .next = OSF_Malloc(sizeof(expr_t)),
        .v.variable = {.name = "msg"}};

    *(_input_fun.v.Native.args[0].next) = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_STRING,
                .String = {
                    .value = "\'\'"}}}};

    int fun_input_mem_idx = PSG_AddFunction(_input_fun);

    fun_t _int_fun = (fun_t){
        .name = "int",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionInt},
        .parent = mod};

    _int_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "num"}};

    int fun_int_mem_idx = PSG_AddFunction(_int_fun);

    fun_t _eval_fun = (fun_t){
        .name = "eval",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionEval},
        .parent = mod};

    _eval_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "_Code"}};

    int fun_eval_mem_idx = PSG_AddFunction(_eval_fun);

    fun_t _nativeType_fun = (fun_t){
        .name = "nativeType",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionnativeType},
        .parent = mod};

    _nativeType_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "_Val"}};

    int fun_nativeType_mem_idx = PSG_AddFunction(_nativeType_fun);

    fun_t _len_fun = (fun_t){
        .name = "len",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionLen},
        .parent = mod};

    _len_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "_Val"}};

    int fun_len_mem_idx = PSG_AddFunction(_len_fun);

    // write(...) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_write_mem_idx, .name = "write"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "write"}}}}, NULL);
    // input(msg) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_input_mem_idx, .name = "input"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "input"}}}}, NULL);
    // int(num) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_int_mem_idx, .name = "int"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "int"}}}}, NULL);
    // eval(_Code) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_eval_mem_idx, .name = "eval"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "eval"}}}}, NULL);
    // nativeType(_Val) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_nativeType_mem_idx, .name = "nativeType"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "nativeType"}}}}, NULL);
    // len(_Val) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_len_mem_idx, .name = "len"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "len"}}}}, NULL);
}

expr_t *Native_Proto_Int__str__(mod_t *mod_ref)
{
    var_t *v_Member = IPSF_GetVar_fromMod(mod_ref, "_Member", NULL);
    assert(v_Member != NULL && SF_FMT("seg_fault"));

    expr_t vmem = v_Member->val;
    assert((
               vmem.type == EXPR_TYPE_CONSTANT &&
               vmem.v.constant.constant_type == CONSTANT_TYPE_INT) &&
           SF_FMT("seg_fault"));

    char *orepr = _IPSF_ObjectRepr(vmem, 0);
    char *resorepr = OSF_Malloc((strlen(orepr) + 3) * sizeof(char));
    sprintf(resorepr, "\'%s\'", orepr);

    OSF_IntFree(orepr);
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_STRING,
                .String = resorepr}}};

    return __res;
}

expr_t *Native_Proto_Array_push(mod_t *mod_ref)
{
    var_t *v_Member = IPSF_GetVar_fromMod(mod_ref, "_Member", NULL),
          *v_Data = IPSF_GetVar_fromMod(mod_ref, "data", NULL);
    assert(v_Member != NULL && SF_FMT("seg_fault"));

    expr_t vmem = v_Member->val,
           vdata = v_Data->val;
    
    assert((
               vmem.type == EXPR_TYPE_CONSTANT &&
               vmem.v.constant.constant_type == CONSTANT_TYPE_ARRAY) &&
           SF_FMT("seg_fault"));

    array_t *_a_ref = PSG_GetArray_Ptr(vmem.v.constant.Array.index);
    Sf_Array_Push(_a_ref, vdata);

    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_DTYPE,
                .DType = {
                    .type = DATA_TYPE_NONE}}}};

    return __res;
}

expr_t *Native_Proto_Array_pop(mod_t *mod_ref)
{
    var_t *v_Member = IPSF_GetVar_fromMod(mod_ref, "_Member", NULL),
          *v_Data = IPSF_GetVar_fromMod(mod_ref, "data", NULL);
    assert(v_Member != NULL && SF_FMT("seg_fault"));

    expr_t vmem = v_Member->val,
           vdata = v_Data->val;
    assert((
               vmem.type == EXPR_TYPE_CONSTANT &&
               vmem.v.constant.constant_type == CONSTANT_TYPE_ARRAY) &&
           SF_FMT("seg_fault"));

    assert((
               vdata.type == EXPR_TYPE_CONSTANT &&
               vdata.v.constant.constant_type == CONSTANT_TYPE_INT) &&
           SF_FMT("Index must be an integer."));

    array_t *_a_ref = PSG_GetArray_Ptr(vmem.v.constant.Array.index);

    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    *__res = Sf_Array_Pop(_a_ref, vdata.v.constant.Int.value);

    return __res;
}

expr_t *Native_Proto_Array__str__(mod_t *mod_ref)
{
    var_t *v_Member = IPSF_GetVar_fromMod(mod_ref, "_Member", NULL);
    assert(v_Member != NULL && SF_FMT("seg_fault"));

    expr_t vmem = v_Member->val;
    assert((
               vmem.type == EXPR_TYPE_CONSTANT &&
               vmem.v.constant.constant_type == CONSTANT_TYPE_ARRAY) &&
           SF_FMT("seg_fault"));

    char *orepr = _IPSF_ObjectRepr(vmem, 0);
    char *resorepr = OSF_Malloc((strlen(orepr) + 3) * sizeof(char));
    sprintf(resorepr, "\'%s\'", orepr);

    OSF_IntFree(orepr);
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_STRING,
                .String = resorepr}}};

    return __res;
}

void SFAdd_Protos_for_built_in_types(void)
{
    // Int configs
    fun_t _native_dtype_int_method__str__ = (fun_t){
        .name = "__str__",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v = {
            .Native = {
                .arg_size = 1,
                .args = OSF_Malloc(sizeof(var_t)),
                .f = Native_Proto_Int__str__}}};

    _native_dtype_int_method__str__.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v = {.variable.name = "_Member"}};

    int _native_dtype_int_method__str__idx = PSG_AddFunction(_native_dtype_int_method__str__);

    AddDtypePrototype(
        CONSTANT_TYPE_INT,
        "__str__",
        (expr_t){
            .type = EXPR_TYPE_FUNCTION,
            .v = {
                .function_s = {
                    .name = "__str__",
                    .index = _native_dtype_int_method__str__idx}}});

    // Array configs
    fun_t _native_dtype_array_method_push = (fun_t){
        .name = "push",
        .is_native = 1,
        .arg_acceptance_count = 2,
        .v = {
            .Native = {
                .arg_size = 2,
                .args = OSF_Malloc(2 * sizeof(var_t)),
                .f = Native_Proto_Array_push}}};

    _native_dtype_array_method_push.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v = {.variable.name = "_Member"}};

    _native_dtype_array_method_push.v.Native.args[1] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v = {.variable.name = "data"}};

    int _native_dtype_array_method_pushidx = PSG_AddFunction(_native_dtype_array_method_push);

    AddDtypePrototype(
        CONSTANT_TYPE_ARRAY,
        "push",
        (expr_t){
            .type = EXPR_TYPE_FUNCTION,
            .v = {
                .function_s = {
                    .name = "push",
                    .index = _native_dtype_array_method_pushidx}}});

    fun_t _native_dtype_array_method_pop = (fun_t){
        .name = "pop",
        .is_native = 1,
        .arg_acceptance_count = -2,
        .v = {
            .Native = {
                .arg_size = 2,
                .args = OSF_Malloc(2 * sizeof(var_t)),
                .f = Native_Proto_Array_pop}}};

    _native_dtype_array_method_pop.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v = {.variable.name = "_Member"}};

    _native_dtype_array_method_pop.v.Native.args[1] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v = {.variable.name = "data"},
        .next = OSF_Malloc(sizeof(expr_t))};

    *(_native_dtype_array_method_pop.v.Native.args[1].next) = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v = {
            .constant = {
                .constant_type = CONSTANT_TYPE_INT,
                .Int = {
                    .value = -1}}}};

    int _native_dtype_array_method_popidx = PSG_AddFunction(_native_dtype_array_method_pop);

    AddDtypePrototype(
        CONSTANT_TYPE_ARRAY,
        "pop",
        (expr_t){
            .type = EXPR_TYPE_FUNCTION,
            .v = {
                .function_s = {
                    .name = "pop",
                    .index = _native_dtype_array_method_popidx}}});

    fun_t _native_dtype_array_method__str__ = (fun_t){
        .name = "__str__",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v = {
            .Native = {
                .arg_size = 1,
                .args = OSF_Malloc(sizeof(var_t)),
                .f = Native_Proto_Array__str__}}};

    _native_dtype_array_method__str__.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v = {.variable.name = "_Member"}};

    int _native_dtype_array_method__str__idx = PSG_AddFunction(_native_dtype_array_method__str__);

    AddDtypePrototype(
        CONSTANT_TYPE_ARRAY,
        "__str__",
        (expr_t){
            .type = EXPR_TYPE_FUNCTION,
            .v = {
                .function_s = {
                    .name = "__str__",
                    .index = _native_dtype_array_method__str__idx}}});
}

const char *read_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        return NULL;
    }

    char *data = (char *)OSF_Malloc(sizeof(char));
    int data_count = 1;

    *data = '\n';

    while (1)
    {
        int c = fgetc(file);
        if (c == EOF)
        {
            break;
        }

        data = (char *)OSF_Realloc(data, sizeof(char) * (data_count + 1));
        data[data_count] = (char)c;
        data_count++;
    }

    data = (char *)OSF_Realloc(data, sizeof(char) * (data_count + 2));
    data[data_count] = '\n';
    data[data_count + 1] = '\0';

    fclose(file);

    const char *data_cpy = OSF_strdup(data);
    OSF_Free(data);
    data = NULL;

    return data_cpy;
}