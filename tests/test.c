#include "../sunflower.h"

// #define SFTESTAST

int main(int argc, char const *argv[])
{
    SF_InitEnv();

    psf_byte_array_t *ast;
    ast = PSF_AST_fromString(read_file("tests/test.sf"));
    PSF_AST_Preprocess_fromByteArray(ast);
    // PSF_AST_print(ast);

    mod_t *mod = SF_CreateModule(MODULE_TYPE_FILE, ast);
    SF_FrameIT_fromAST(mod);
    
    SFAdd_Protos_for_built_in_types();
    SFBuiltIn_AddDefaultFunctions(mod);

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