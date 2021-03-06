#include "../sunflower.h"

// #define SFTESTAST

int main(int argc, char const *argv[])
{
    if (argc < 2)
        return printf("Usage: test_exe <file_name>\n");

    SF_InitEnv();
    OSF_cmd_set_flag(OSF_cmd_flag_new(CMD_FLAG_DETAILED_ERRORS, NULL, 0));

    psf_byte_array_t *ast;
    char *fd = (char *)read_file(argv[1]);
    OSF_SetFileData(fd);
    OSF_SetFileName((char *)argv[1]);

    ast = PSF_AST_fromString(fd);

    if (OSF_GetExceptionState())
    {
        except_t *expr_log = OSF_GetExceptionLog();
        OSF_RaiseExceptionMessage(expr_log);

        exit(EXIT_FAILURE);
    }

    PSF_AST_Preprocess_fromByteArray(ast);
    // PSF_AST_print(ast);

    if (OSF_GetExceptionState())
    {
        except_t *expr_log = OSF_GetExceptionLog();
        OSF_RaiseExceptionMessage(expr_log);

        exit(EXIT_FAILURE);
    }

    mod_t *mod = SF_CreateModule(MODULE_TYPE_FILE, ast);
    SF_FrameIT_fromAST(mod);
    // mod->path_prefix = _IPSF_GetDir_FromFilePath("tests/test.sf");

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
        else if (mod->v.file_s.body[i].type == STATEMENT_TYPE_IF)
        {
            printf("\t");

            for (size_t j = 0; j < mod->v.file_s.body[i].v.if_stmt.body_size; j++)
            {
                PSG_PrintStatementType(mod->v.file_s.body[i].v.if_stmt.body[j].type);
                printf("\t");
            }
        }
    }
#else
    int err;
    OSF_Free(IPSF_ExecIT_fromMod(mod, &err));

    if (err != IPSF_OK)
    {
        switch (err)
        {
        case IPSF_NOT_OK_CHECK_EXPR_LOG:
        {
            except_t *expr_log = OSF_GetExceptionLog();
            OSF_RaiseExceptionMessage(expr_log);
        }
        break;

        default:
            break;
        }

        exit(EXIT_FAILURE);
    }
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