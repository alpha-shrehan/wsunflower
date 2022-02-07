#include "osf_except.h"
#include "osf_cmd.h"

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

except_t *exc_log; // Size 1
int exception_occupied;
char *file_data;
char *file_path;
backtrace_t *bck_log;
int bck_lcount;

void OSF_ExceptionInit(void)
{
    exc_log = OSF_Malloc(sizeof(except_t));
    exception_occupied = 0;
    file_data = OSF_Malloc(1);
    *file_data = '\0';
    file_path = OSF_Malloc(1);
    *file_path = '\0';
    bck_log = OSF_Malloc(sizeof(backtrace_t));
    bck_lcount = 0;
}

int OSF_GetExceptionState(void)
{
    return exception_occupied;
}

void OSF_SetExceptionState(int v)
{
    exception_occupied = v;
}

void OSF_SetException(except_t e)
{
    *exc_log = e;
    OSF_SetExceptionState(1);
}

except_t *OSF_GetExceptionLog(void)
{
    return exc_log;
}

void OSF_RaiseExceptionMessage(except_t *e)
{
    char **spl_fd = Sf_Str_Split(OSF_GetFileData(), "\n");
    int bl = *OSF_GetBacklogSize();
    backtrace_t *bb = *OSF_GetBacklog();
    clopt_t *flg_extra_info = OSF_cmd_get_flag_fromType(CMD_FLAG_DETAILED_ERRORS);

#if defined(WIN32) || defined(_WIN32)
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attrs;

    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    saved_attrs = consoleInfo.wAttributes;

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    printf("Error: ");

    SetConsoleTextAttribute(hConsole, saved_attrs);
#else
    printf("Error: ");
#endif

    printf("In file '%s',\n", OSF_GetFileName());

    switch (e->type)
    {
    case EXCEPT_INDEX_OUT_OF_RANGE:
    {
        printf("       Requested index out of range.\n");

        if (flg_extra_info != NULL)
        {
            printf("\nAdditional meta-data returned by the interpreter:\n");
            printf("[Index requested   : %d]\n", e->v.ce0.idx);
            printf("[Range length      : %d]\n", e->v.ce0.targ.len);
            printf("[Exceeded limit by : %d]\n", e->v.ce0.idx - e->v.ce0.targ.len);
        }
    }
    break;
    case EXCEPT_VARIABLE_DOES_NOT_EXIST:
    {
        printf("       Variable '%s' does not exist.\n", e->v.ce1.vname);

        if (flg_extra_info != NULL)
        {
            printf("\nAdditional meta-data returned by the interpreter:\n");
            printf("[Variable referenced : '%s']\n", e->v.ce1.vname);

            if (e->v.ce1.m_ref != NULL)
            {
                int closest_score = INT_MAX;
                char *cv = NULL;

                for (size_t i = 0; i < e->v.ce1.m_ref->var_holds_size; i++)
                {
                    int x = strcmp(e->v.ce1.vname, e->v.ce1.m_ref->var_holds[i].name);
                    if (x < closest_score &&
                        strlen(e->v.ce1.vname) ==
                            strlen(e->v.ce1.m_ref->var_holds[i].name))
                    {
                        closest_score = x;
                        cv = e->v.ce1.m_ref->var_holds[i].name;
                    }
                }

                if (cv != NULL)
                    printf("[Closest Match       : '%s']\n", cv);
            }
        }
    }
    break;
    case EXCEPT_SYNTAX_ERROR:
    {
        printf("       Syntax Error.\n");

        if (flg_extra_info != NULL)
        {
            if (e->v.ce10.additional_msg != NULL)
            {
                printf("\nAdditional meta-data returned by the interpreter:\n");
                if (e->v.ce10.additional_msg != NULL)
                    printf("[%s]\n", e->v.ce10.additional_msg);
            }
        }
    }
    break;
    case EXCEPT_IMPORTRED_FILE_NOT_FOUND:
    {
        printf("       Imported file not found.\n");

        if (flg_extra_info != NULL)
        {
            if (e->v.ce31.file_name != NULL)
            {
                printf("\nAdditional meta-data returned by the interpreter:\n");
                printf("[Module: %s]\n", e->v.ce31.file_name);
            }

            if (e->v.ce31.paths != NULL)
            {
                printf("\nFiles searched: \n");
                while (*(e->v.ce31.paths) != NULL)
                    printf("['%s']\n", *(e->v.ce31.paths++));
            }
        }
    }
    break;
    case EXCEPT_CODE_ASSERTION_ERROR:
    {
        printf("       Assertion Error");

        if (e->v.ce11.msg == NULL)
            printf(".\n");
        else
            printf(": %s\n", e->v.ce11.msg);
    }
    break;
    case EXCEPT_CLASS_HAS_NO_CONSTRUCTOR:
    {
        printf("       Class has no constructor.\n");

        if (flg_extra_info != NULL)
        {
            printf("\nAdditional meta-data returned by the interpreter:\n");
            if (e->v.ce12.class_ref != NULL)
                printf("[<class '%s'>]\n", e->v.ce12.class_ref->name);
        }
    }
    break;
    case EXCEPT_ENTITY_IS_NOT_A_FUNCTION:
    {
        printf("       Entity is not a function.\n");

        if (flg_extra_info != NULL)
        {
            printf("\nAdditional meta-data returned by the interpreter:\n");
            if (e->v.ce3.v_ref != NULL)
                printf("[Variable Name: '%s']\n", e->v.ce3.v_ref->name);
        }
    }
    break;
    case EXCEPT_RETURN_CALLED_OUTSIDE_FUNCTION:
    {
        printf("       return called outside function.\n");
    }
    break;
    case EXCEPT_CLASS_OBJ_NOT_AN_ITERABLE:
    {
        printf("       Class object is not an iterable.\n");
    }
    break;
    case EXCEPT_ITERATIVE_MUST_BE_A_CLASS_OBJECT:
    {
        printf("       Iterative must be a class object.\n");
    }
    break;
    case EXCEPT_CLASS_OBJ_NOT_AN_ITERATIVE:
    {
        printf("       Class object is not an iterative.\n");
    }
    break;
    case EXCEPT_DICT_ITERATION_ALLOWS_MAX_2_SUBSTITUTION_VARS:
    {
        printf("       Dictionary iteration allows a maximum of 2 substitution variables.\n");
    }
    break;
    case EXCEPT_ITERATION_COUNT_MUST_BE_AN_INTEGER:
    {
        printf("       Iteration count must be an integer.\n");
    }
    break;
    case EXCEPT_IMPORT_MUST_HAVE_AN_ALIAS:
    {
        printf("       Import module must have an alias.\n");
    }
    break;
    case EXCEPT_MODULE_HAS_NO_CONSTRUCTOR:
    {
        printf("       Module has no constructor.\n");
    }
    break;
    case EXCEPT_MODULE_CONSTRUCTOR_IS_NOT_A_FUNCTION:
    {
        printf("       Module constructor is not a function.\n");
    }
    break;
    case EXCEPT_INLINE_ASSIGNMENT_IS_NOT_ALLOWED_IN_MODULE_CONSTRUCTOR:
    {
        printf("       Inline assignment is not allowed in module constructor.\n");
    }
    break;
    case EXCEPT_STEP_COUNT_MUST_BE_AN_INTEGER:
    {
        printf("       Step count must be an integer.\n");
    }
    break;
    case EXCEPT_TO_STEP_ENTITIES_MUST_BE_AN_INTEGER:
    {
        printf("       To step entities must be an integer.\n");
    }
    break;
    case EXCEPT_CLASS_OBJECT_IS_NOT_CALLABLE:
    {
        printf("       Class object is not callable.\n");
    }
    break;
    case EXCEPT_CLASS_OBJECT_CANNOT_BE_INDEXED:
    {
        printf("       Class object cannot be indexed.\n");
    }
    break;
    case EXCEPT_OBJECT_HAS_NO_MEMBER:
    {
        printf("       Object has no member.\n");
    }
    break;
    case EXCEPT_MODULE_HAS_NO_MEMBER:
    {
        printf("       Module has no member.\n");
    }
    break;
    case EXCEPT_CANNOT_OVERLOAD_DOT_ON_ENTITY:
    {
        printf("       Cannot overload operator '.' on entity.\n");
    }
    break;
    case EXCEPT_ARRAY_INDEX_MUST_BE_AN_INTEGER:
    {
        printf("       Array index must be an integer.\n");
    }
    break;
    case EXCEPT_CANNOT_USE_UNARY_MINUS_ON_NON_NUMBER_ENTITY:
    {
        printf("       Cannot use unary '-' on non number entity.\n");
    }
    break;
    case EXCEPT_INVALID_USAGE_OF_OP_PLUS:
    {
        printf("       Invalid usage of unary '+'.\n");
    }
    break;
    case EXCEPT_INVALID_NUMBER_OF_ARGS_PASSED:
    {
        printf("       Invalid number of arguments passed.\n");
    }
    break;
    case EXCEPT_FUNCTION_WITH_VARIABLE_ARGUMENTS_NEEDS_TO_HAVE_AN_UNDEFINED_VARIABLE:
    {
        printf("       Function with variable arguments needs to have an undefined variable.\n");
    }
    break;
    case EXCEPT_UNKNOWN_ENTITIES_TO_COMPARE:
    {
        printf("       Unknown entities to compare.\n");
    }
    break;
    case EXCEPT_ENTITY_MUST_BE_A_STRING:
    {
        printf("       Entity must be a string.\n");
    }
    break;
    case EXCEPT_INVALID_ITERATOR:
    {
        printf("       Invalid iterator.\n");
    }
    break;
    case EXCEPT_NON_DEFAULT_ARGUMENT_FOLLOWS_A_DEFAULT_ARG:
    {
        printf("       Non default argument follows a default arg.\n");
    }
    break;
    case EXCEPT_NON_DEFAULT_ARGUMENT_FOLLOWS_VAR_ARGS:
    {
        printf("       Non default argument follows variable arguments.\n");
    }
    break;
    case EXCEPT_STRING_IS_IMMUTABLE:
    {
        printf("       String is immutable.\n");
    }
    break;
    default:
        printf("       Asynchronous exception not defined yet. (Code: %d)\n", e->type);
        break;
    }

    printf("\n");

    if (bl)
        if (flg_extra_info != NULL)
        {
            printf("-------------\n");
            printf("| Backtrace |\n");
            printf("-------------\n");

            for (int i = bl - 1; i >= 0; i--)
            {
                // TODO: here
                printf("%d | %s\n", bb[i].line - 1, spl_fd[bb[i].line - 1]);
            }
        }
        else
        {
            /* Print last line */
            int std_l = printf("%d | %s\n", bb[bl - 1].line - 1, spl_fd[bb[bl - 1].line - 1]);
            for (size_t j = 0; j < std_l; j++)
                printf("-");
            printf("\n");
        }
    else if (e->line - 1 > 0)
        printf("%d | %s\n", e->line - 1, spl_fd[e->line - 1]);
}

void OSF_SetFileData(char *d)
{
    OSF_Free(file_data);
    file_data = OSF_strdup(d);
}

char *OSF_GetFileData(void)
{
    return file_data;
}

void OSF_SetFileName(char *c)
{
    OSF_Free(c);
    file_path = OSF_strdup(c);
}

char *OSF_GetFileName(void)
{
    return file_path;
}

void OSF_AddBackLog(backtrace_t b)
{
    if (!b.line)
        return;

    if (*OSF_GetBacklogSize())
    {
        if ((*OSF_GetBacklog())[(*OSF_GetBacklogSize()) - 1].line == b.line)
            return;
        (*OSF_GetBacklog()) = OSF_Realloc(*OSF_GetBacklog(), (*OSF_GetBacklogSize() + 1) * sizeof(backtrace_t));
    }

    (*OSF_GetBacklog())[(*OSF_GetBacklogSize())++] = b;
}

void OSF_PopBackLog(void)
{
    if (!(*OSF_GetBacklogSize()))
        return;

    int bl_size = *OSF_GetBacklogSize();
    backtrace_t *cpy = OSF_Malloc((bl_size - 1) * sizeof(backtrace_t));

    for (size_t i = 0; i < bl_size - 1; i++)
        cpy[i] = (*OSF_GetBacklog())[i];

    OSF_Free(*OSF_GetBacklog());
    *OSF_GetBacklog() = cpy;
    (*OSF_GetBacklogSize())--;
}

backtrace_t **OSF_GetBacklog(void)
{
    return &bck_log;
}

int *OSF_GetBacklogSize(void)
{
    return &bck_lcount;
}

void OSF_ClearBacklog(void)
{
    OSF_Free(*OSF_GetBacklog());
    *OSF_GetBacklog() = OSF_Malloc(sizeof(backtrace_t));
    *OSF_GetBacklogSize() = 0;
}

backtrace_t OSF_CreateBackLog(int type, int line)
{
    backtrace_t b;
    b.type = type;
    b.line = line;

    return b;
}

void OSF_RaiseException_SyntaxError(int line, char *msg)
{
    OSF_SetException((except_t){
        .type = EXCEPT_SYNTAX_ERROR,
        .line = line,
        .v.ce10.additional_msg = msg});
}

void OSF_RaiseException_ImportFileDNE(int line, char *fname, char **pats)
{
    OSF_SetException((except_t){
        .type = EXCEPT_IMPORTRED_FILE_NOT_FOUND,
        .line = line,
        .v.ce31 = {
            .file_name = fname,
            .paths = pats}});
}

void OSF_RaiseException_CodeAssertionError(int line, char *msg)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CODE_ASSERTION_ERROR,
        .line = line,
        .v.ce11 = {
            .msg = msg}});
}

void OSF_RaiseException_ClassHasNoConstructor(int line, class_t *c_ref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CLASS_HAS_NO_CONSTRUCTOR,
        .line = line,
        .v.ce12.class_ref = c_ref});
}

void OSF_RaiseException_EntityIsNotAFunction(int line, var_t *v_ref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_ENTITY_IS_NOT_A_FUNCTION,
        .line = line,
        .v.ce3.v_ref = v_ref});
}

void OSF_RaiseException_ReturnCalledOutsideFunction(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_RETURN_CALLED_OUTSIDE_FUNCTION,
        .line = line});
}

void OSF_RaiseException_VariableDoesNotExist(int line, mod_t *mref, char *vname)
{
    OSF_SetException((except_t){
        .type = EXCEPT_VARIABLE_DOES_NOT_EXIST,
        .line = line,
        .v.ce1 = {
            .m_ref = mref,
            .vname = vname}});
}

void OSF_RaiseException_IndexOutOfRange(int line, int idx, array_t ref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_INDEX_OUT_OF_RANGE,
        .line = line,
        .v.ce0 = {
            .idx = idx,
            .targ = ref}});
}

void OSF_RaiseException_ClassObjectIsNotAnIterable(int line, class_t *c_ref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CLASS_OBJ_NOT_AN_ITERABLE,
        .line = line,
        .v.ce2.c_ref = c_ref});
}

void OSF_RaiseException_IterativeMustBeAClassObject(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_ITERATIVE_MUST_BE_A_CLASS_OBJECT,
        .line = line});
}

void OSF_RaiseException_ClassObjectIsNotAnIterative(int line, class_t *c_ref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CLASS_OBJ_NOT_AN_ITERATIVE,
        .line = line,
        .v.ce5.c_ref = c_ref});
}

void OSF_RaiseException_DictIterationAllowsMax2SubstitutionVars(int line, int vpr)
{
    OSF_SetException((except_t){
        .type = EXCEPT_DICT_ITERATION_ALLOWS_MAX_2_SUBSTITUTION_VARS,
        .line = line,
        .v.ce6.vars_provided = vpr});
}

void OSF_RaiseException_IterationCountMustBeAnInteger(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_ITERATION_COUNT_MUST_BE_AN_INTEGER,
        .line = line});
}

void OSF_RaiseException_ImportMustHaveAnAlias(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_IMPORT_MUST_HAVE_AN_ALIAS,
        .line = line});
}

void OSF_RaiseException_ModuleHasNoConstructor(int line, mod_t *mref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_MODULE_HAS_NO_CONSTRUCTOR,
        .line = line,
        .v.ce13.mod_ref = mref});
}

void OSF_RaiseException_ModuleConstructorIsNotAFunction(int line, var_t *vref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_MODULE_CONSTRUCTOR_IS_NOT_A_FUNCTION,
        .line = line,
        .v.ce14.var_ref = vref});
}

void OSF_RaiseException_InlineAssignmentIsNotAllowedInModuleConstructors(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_INLINE_ASSIGNMENT_IS_NOT_ALLOWED_IN_MODULE_CONSTRUCTOR,
        .line = line});
}

void OSF_RaiseException_StepCountMustBeAnInteger(int line, expr_t eg)
{
    OSF_SetException((except_t){
        .type = EXCEPT_STEP_COUNT_MUST_BE_AN_INTEGER,
        .line = line,
        .v.ce16.ent_got = eg});
}

void OSF_RaiseException_ToStepEntitiesMustBeAnInteger(int line, expr_t e1, expr_t e2)
{
    OSF_SetException((except_t){
        .type = EXCEPT_TO_STEP_ENTITIES_MUST_BE_AN_INTEGER,
        .line = line,
        .v.ce17 = {
            .lhs = e1,
            .rhs = e2}});
}

void OSF_RaiseException_ClassObjectIsNotCallable(int line, class_t *cref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CLASS_OBJECT_IS_NOT_CALLABLE,
        .line = line,
        .v.ce18.c_ref = cref});
}

void OSF_RaiseException_ClassObjectCannotBeIndexed(int line, class_t *cref)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CLASS_OBJECT_CANNOT_BE_INDEXED,
        .line = line,
        .v.ce19.c_ref = cref});
}

void OSF_RaiseException_ObjectHasNoMember(int line, expr_t e, char *mem)
{
    OSF_SetException((except_t){
        .type = EXCEPT_OBJECT_HAS_NO_MEMBER,
        .line = line,
        .v.ce20 = {
            .member = mem,
            .obj = e}});
}

void OSF_RaiseException_ModuleHasNoMember(int line, mod_t *mref, char *name)
{
    OSF_SetException((except_t){
        .type = EXCEPT_MODULE_HAS_NO_MEMBER,
        .line = line,
        .v.ce21 = {
            .member = name,
            .mod = mref}});
}

void OSF_RaiseException_CannotOverloadDotOnEntity(int line, expr_t ent)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CANNOT_OVERLOAD_DOT_ON_ENTITY,
        .line = line,
        .v.ce22.entity = ent});
}

void OSF_RaiseException_ArrayIndexMustBeAnInteger(int line, expr_t ent)
{
    OSF_SetException((except_t){
        .type = EXCEPT_ARRAY_INDEX_MUST_BE_AN_INTEGER,
        .line = line,
        .v.ce23.index = ent});
}

void OSF_RaiseException_CannotUseUnaryMinusOnNonNumberEntity(int line, expr_t ent)
{
    OSF_SetException((except_t){
        .type = EXCEPT_CANNOT_USE_UNARY_MINUS_ON_NON_NUMBER_ENTITY,
        .line = line,
        .v.ce24.ent = ent});
}

void OSF_RaiseException_InvalidUsageOfOpPlus(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_INVALID_USAGE_OF_OP_PLUS,
        .line = line});
}

void OSF_RaiseException_InvalidNumberOfArgsPassed(int line, int ap, int ae)
{
    OSF_SetException((except_t){
        .type = EXCEPT_INVALID_NUMBER_OF_ARGS_PASSED,
        .line = line,
        .v.ce26 = {
            .args_passed = ap,
            .args_expected = ae}});
}

void OSF_RaiseException_FunctionWithVaArgsNeedsToHaveAnUndefinedVariable(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_FUNCTION_WITH_VARIABLE_ARGUMENTS_NEEDS_TO_HAVE_AN_UNDEFINED_VARIABLE,
        .line = line});
}

void OSF_RaiseException_UnknownEntitiesToCompare(int line, expr_t e1, expr_t e2)
{
    OSF_SetException((except_t){
        .type = EXCEPT_UNKNOWN_ENTITIES_TO_COMPARE,
        .line = line,
        .v.ce28 = {
            .ent1 = e1,
            .ent2 = e2}});
}

void OSF_RaiseException_EntityMustBeAString(int line, expr_t ent)
{
    OSF_SetException((except_t){
        .type = EXCEPT_ENTITY_MUST_BE_A_STRING,
        .line = line,
        .v.ce29.ent = ent});
}

void OSF_RaiseException_InvalidIterator(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_INVALID_ITERATOR,
        .line = line});
}

void OSF_RaiseException_NonDefaultArgFollowsADefaultArg(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_NON_DEFAULT_ARGUMENT_FOLLOWS_A_DEFAULT_ARG,
        .line = line});
}

void OSF_RaiseException_NonDefaultArgFollowsVarArgs(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_NON_DEFAULT_ARGUMENT_FOLLOWS_VAR_ARGS,
        .line = line});
}

void OSF_RaiseException_InheritMustBeAClass(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_INHERIT_MUST_BE_A_CLASS,
        .line = line});
}

void OSF_RaiseException_StringIsImmutable(int line)
{
    OSF_SetException((except_t){
        .type = EXCEPT_STRING_IS_IMMUTABLE,
        .line = line});
}