#include "../sunflower.h"

// #define SFTESTAST

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

    const char *data_cpy = strdup(data);
    OSF_Free(data);
    data = NULL;

    return data_cpy;
}

expr_t *NativeFunctionWrite(mod_t *mod_ref)
{
    expr_t *__res = OSF_Malloc(sizeof(expr_t));
    var_t *args_v = IPSF_GetVar_fromMod(mod_ref, "args", NULL);
    assert((args_v->val.type == EXPR_TYPE_CONSTANT && args_v->val.v.constant.constant_type == CONSTANT_TYPE_ARRAY) && SF_FMT("Error: `args' must be an array"));
    expr_t *args = args_v->val.v.constant.Array.vals;
    int arg_count = args_v->val.v.constant.Array.len;

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

                    char *x = _IPSF_ObjectRepr(*_v_res, 0);
                    printf("%s", x);

                    OSF_Free(_v_res);
                    OSF_IntFree(x);
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

    *__res = (expr_t){
        .type = EXPR_TYPE_CONSTANT,
        .v.constant = {
            .constant_type = CONSTANT_TYPE_DTYPE,
            .DType = {
                .type = DATA_TYPE_NONE}}};

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

    const char *_f_dup = strdup(_fdata);
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
           SF_FMT("Error: `num' must be a string"));

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

// expr_t *NativeFunctionEmpty(mod_t *mod_ref)
// {
//     expr_t *r = OSF_Malloc(sizeof(expr_t));
//     *r = (expr_t){
//         .type = EXPR_TYPE_CONSTANT,
//         .v = {
//             .constant = {
//                 .constant_type = CONSTANT_TYPE_INT,
//                 .Int = {
//                     .value = 0}}}};

//     return r;
// }

int main(int argc, char const *argv[])
{
    SF_InitEnv();

    fun_t _write_fun = (fun_t){
        .name = "write",
        .is_native = 1,
        .arg_acceptance_count = -1,
        .v.Native = {
            .arg_size = 3,
            .args = OSF_Malloc(3 * sizeof(expr_t)),
            .f = NativeFunctionWrite}};

    _write_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "args"}};

    _write_fun.v.Native.args[1] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .next = (expr_t *)(expr_t[]){
            (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v = {
                    .constant = {
                        .constant_type = CONSTANT_TYPE_STRING,
                        .String = {
                            .value = "\"\n\""}}}}},
        .v.variable = {.name = "end"}};

    _write_fun.v.Native.args[2] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .next = (expr_t *)(expr_t[]){
            (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v = {
                    .constant = {
                        .constant_type = CONSTANT_TYPE_STRING,
                        .String = {
                            .value = "\" \""}}}}},
        .v.variable = {.name = "join"}};

    int fun_write_mem_idx = PSG_AddFunction(_write_fun);

    fun_t _input_fun = (fun_t){
        .name = "input",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionInput}};

    _input_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .next = (expr_t *)(expr_t[]){
            (expr_t){
                .type = EXPR_TYPE_CONSTANT,
                .v = {
                    .constant = {
                        .constant_type = CONSTANT_TYPE_STRING,
                        .String = {
                            .value = "\'\'"}}}}},
        .v.variable = {.name = "msg"}};

    int fun_input_mem_idx = PSG_AddFunction(_input_fun);

    fun_t _int_fun = (fun_t){
        .name = "int",
        .is_native = 1,
        .arg_acceptance_count = 1,
        .v.Native = {
            .arg_size = 1,
            .args = OSF_Malloc(sizeof(expr_t)),
            .f = NativeFunctionInt}};

    _int_fun.v.Native.args[0] = (expr_t){
        .type = EXPR_TYPE_VARIABLE,
        .v.variable = {.name = "num"}};

    int fun_int_mem_idx = PSG_AddFunction(_int_fun);

    // fun_t _empty_fun = (fun_t){
    //     .name = "empty",
    //     .is_native = 1,
    //     .arg_acceptance_count = 1,
    //     .v.Native = {
    //         .arg_size = 1,
    //         .args = OSF_Malloc(sizeof(expr_t)),
    //         .f = NativeFunctionEmpty}};

    // _empty_fun.v.Native.args[0] = (expr_t){
    //     .type = EXPR_TYPE_VARIABLE,
    //     .v.variable = {.name = "test"}};

    // int fun_empty_mem_idx = PSG_AddFunction(_empty_fun);

    psf_byte_array_t *ast;
    ast = PSF_AST_fromString(read_file("tests/test.sf"));
    PSF_AST_Preprocess_fromByteArray(ast);
    // PSF_AST_print(ast);

    mod_t *mod = SF_CreateModule(MODULE_TYPE_FILE, ast);
    SF_FrameIT_fromAST(mod);

    // write(...) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_write_mem_idx, .name = "write"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "write"}}}}, NULL);
    // input(msg) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_input_mem_idx, .name = "input"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "input"}}}}, NULL);
    // int(num) function
    IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_int_mem_idx, .name = "int"}}}, .name = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_VARIABLE, .v.variable.name = "int"}}}}, NULL);
    // empty() function
    // IPSF_ExecVarDecl_fromStmt(mod, (stmt_t){.type = STATEMENT_TYPE_VAR_DECL, .v.var_decl = {.expr = (expr_t *)(expr_t[]){(expr_t){.type = EXPR_TYPE_FUNCTION, .v.function_s = {.index = fun_empty_mem_idx, .name = "empty"}}}, .name = "empty"}}, NULL);

#ifdef SFTESTAST
    for (size_t i = 0; i < mod->v.file_s.body_size; i++)
    {
        PSG_PrintStatementType(mod->v.file_s.body[i].type);

        if (mod->v.file_s.body[i].type == STATEMENT_TYPE_EXPR)
        {
            printf("\t");
            PSG_PrintExprType(mod->v.file_s.body[i].v.expr.expr->type);

            if (mod->v.file_s.body[i].v.expr.expr->type == EXPR_TYPE_FUNCTION_CALL)
            {
                for (size_t j = 0; j < mod->v.file_s.body[i].v.expr.expr->v.function_call.arg_size; j++)
                {
                    printf("\t\t");
                    PSG_PrintExprType(mod->v.file_s.body[i].v.expr.expr->v.function_call.args[j].type);
                }
            }
        }
        else if (mod->v.file_s.body[i].type == STATEMENT_TYPE_VAR_DECL)
        {
            printf("\t");
            PSG_PrintExprType(mod->v.file_s.body[i].v.var_decl.expr->type);

            if (mod->v.file_s.body[i].v.expr.expr->type == EXPR_TYPE_FUNCTION_CALL)
            {
                for (size_t j = 0; j < mod->v.file_s.body[i].v.expr.expr->v.function_call.arg_size; j++)
                {
                    printf("\t\t");
                    PSG_PrintExprType(mod->v.file_s.body[i].v.expr.expr->v.function_call.args[j].type);
                }
            }
        }
    }
#else
    int err;
    OSF_Free(IPSF_ExecIT_fromMod(mod, &err));

    assert(err == IPSF_OK);
#endif

    // printf("%d\n", *(int *) SF_CCast_Entity(IPSF_GetVar_fromMod(mod, "abc", NULL)->val));
    // printf("%s\n", (const char *) SF_CCast_Entity(IPSF_GetVar_fromMod(mod, "def", NULL)->val));
    // printf("%.2f\n", *(float *) SF_CCast_Entity(IPSF_GetVar_fromMod(mod, "ghi", NULL)->val));
    // printf("%s\n", SF_CCast_Entity(IPSF_GetVar_fromMod(mod, "jkl", NULL)->val) == NULL ? "NULL" : "NOT NULL");
    // printf("%d\n", *(int *) SF_CCast_Entity(IPSF_GetVar_fromMod(mod, "mno", NULL)->val));

    _IPSF_DestClasses(mod);
    SF_Module_safeDelete(mod);
    OSF_Free(mod);
    SF_DestroyEnv();
    return 0;
}