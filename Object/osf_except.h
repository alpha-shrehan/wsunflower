#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Parser/str.h>

#include <Parser/psf_gen_inst_ast.h>

enum SFExceptionMessageCode
{
    EXCEPT_INDEX_OUT_OF_RANGE = 0,
    EXCEPT_VARIABLE_DOES_NOT_EXIST = 1,
    EXCEPT_CLASS_OBJ_NOT_AN_ITERABLE = 2,
    EXCEPT_ENTITY_IS_NOT_A_FUNCTION = 3,
    EXCEPT_ITERATIVE_MUST_BE_A_CLASS_OBJECT = 4,
    EXCEPT_CLASS_OBJ_NOT_AN_ITERATIVE = 5,
    EXCEPT_DICT_ITERATION_ALLOWS_MAX_2_SUBSTITUTION_VARS = 6,
    EXCEPT_ITERATION_COUNT_MUST_BE_AN_INTEGER = 7,
    EXCEPT_RETURN_CALLED_OUTSIDE_FUNCTION = 8,
    EXCEPT_IMPORT_MUST_HAVE_AN_ALIAS = 9,
    EXCEPT_SYNTAX_ERROR = 10,
    EXCEPT_CODE_ASSERTION_ERROR = 11,
    EXCEPT_CLASS_HAS_NO_CONSTRUCTOR = 12,
    EXCEPT_MODULE_HAS_NO_CONSTRUCTOR = 13,
    EXCEPT_MODULE_CONSTRUCTOR_IS_NOT_A_FUNCTION = 14,
    EXCEPT_INLINE_ASSIGNMENT_IS_NOT_ALLOWED_IN_MODULE_CONSTRUCTOR = 15,
    EXCEPT_STEP_COUNT_MUST_BE_AN_INTEGER = 16,
    EXCEPT_TO_STEP_ENTITIES_MUST_BE_AN_INTEGER = 17,
    EXCEPT_CLASS_OBJECT_IS_NOT_CALLABLE = 18,
    EXCEPT_CLASS_OBJECT_CANNOT_BE_INDEXED = 19,
    EXCEPT_OBJECT_HAS_NO_MEMBER = 20,
    EXCEPT_MODULE_HAS_NO_MEMBER = 21,
    EXCEPT_CANNOT_OVERLOAD_DOT_ON_ENTITY = 22,
    EXCEPT_ARRAY_INDEX_MUST_BE_AN_INTEGER = 23,
    EXCEPT_CANNOT_USE_UNARY_MINUS_ON_NON_NUMBER_ENTITY = 24,
    EXCEPT_INVALID_USAGE_OF_OP_PLUS = 25,
    EXCEPT_INVALID_NUMBER_OF_ARGS_PASSED = 26,
    EXCEPT_FUNCTION_WITH_VARIABLE_ARGUMENTS_NEEDS_TO_HAVE_AN_UNDEFINED_VARIABLE = 27,
    EXCEPT_UNKNOWN_ENTITIES_TO_COMPARE = 28,
    EXCEPT_ENTITY_MUST_BE_A_STRING = 29,
    EXCEPT_INVALID_ITERATOR = 30,
    EXCEPT_IMPORTRED_FILE_NOT_FOUND = 31,
    EXCEPT_NON_DEFAULT_ARGUMENT_FOLLOWS_A_DEFAULT_ARG = 32,
    EXCEPT_NON_DEFAULT_ARGUMENT_FOLLOWS_VAR_ARGS = 33,
    EXCEPT_INHERIT_MUST_BE_A_CLASS = 34
};

struct _except_s
{
    enum SFExceptionMessageCode type;
    int line;

    union
    {
        struct
        {
            int idx;
            array_t targ;
        } ce0;

        struct
        {
            char *vname;
            mod_t *m_ref; // For 'Did you mean <var>?'
        } ce1;

        struct
        {
            class_t *c_ref;
        } ce2;

        struct
        {
            var_t *v_ref;
        } ce3;

        struct
        {
            int algn;
        } ce4;

        struct
        {
            class_t *c_ref;
        } ce5;

        struct
        {
            int vars_provided;
        } ce6;

        struct
        {
            int algn;
        } ce7;

        struct
        {
            int algn;
        } ce8;

        struct
        {
            int algn;
        } ce9;

        struct
        {
            char *additional_msg;
        } ce10;

        struct
        {
            char *msg;
        } ce11;

        struct
        {
            class_t *class_ref;
        } ce12;

        struct
        {
            mod_t *mod_ref;
        } ce13;

        struct
        {
            var_t *var_ref;
        } ce14;

        struct
        {
            int algn;
        } ce15;

        struct
        {
            expr_t ent_got;
        } ce16;

        struct
        {
            expr_t lhs;
            expr_t rhs;
        } ce17;

        struct
        {
            class_t *c_ref;
        } ce18;

        struct
        {
            class_t *c_ref;
        } ce19;

        struct
        {
            expr_t obj;
            char *member;
        } ce20;

        struct
        {
            mod_t *mod;
            char *member;
        } ce21;

        struct
        {
            expr_t entity;
        } ce22;

        struct
        {
            expr_t index;
        } ce23;

        struct
        {
            expr_t ent;
        } ce24;

        struct
        {
            int algn;
        } ce25;

        struct
        {
            int args_passed;
            int args_expected;
        } ce26;

        struct
        {
            int algn;
        } ce27;

        struct
        {
            expr_t ent1;
            expr_t ent2;
        } ce28;

        struct
        {
            expr_t ent;
        } ce29;

        struct
        {
            int algn;
        } ce30;

        struct
        {
            char *file_name;
            char **paths;
        } ce31;

        struct
        {
            int algn;
        } ce32;

        struct
        {
            int algn;
        } ce33;

        struct
        {
            int algn;
        } ce34;

    } v;
};

typedef struct _except_s except_t;

enum BacktraceEntityType
{
    BACKTRACE_ENTITY_EXPR,
    BACKTRACE_ENTITY_STMT
};

struct _backtr_s
{
    enum BacktraceEntityType type;
    int line;
};

typedef struct _backtr_s backtrace_t;

void OSF_ExceptionInit(void);
void OSF_SetException(except_t);
int OSF_GetExceptionState(void);
void OSF_SetExceptionState(int);
except_t *OSF_GetExceptionLog(void);
void OSF_RaiseExceptionMessage(except_t *);
void OSF_SetFileData(char *);
char *OSF_GetFileData(void);
void OSF_SetFileName(char *);
char *OSF_GetFileName(void);
void OSF_AddBackLog(backtrace_t);
void OSF_PopBackLog(void);
backtrace_t **OSF_GetBacklog(void);
int *OSF_GetBacklogSize(void);
void OSF_ClearBacklog(void);
backtrace_t OSF_CreateBackLog(int, int);

// Exception routines
void OSF_RaiseException_VariableDoesNotExist(int, mod_t *, char *);
void OSF_RaiseException_SyntaxError(int, char *);
void OSF_RaiseException_ImportFileDNE(int, char *, char **);
void OSF_RaiseException_CodeAssertionError(int, char *);
void OSF_RaiseException_ClassHasNoConstructor(int, class_t *);
void OSF_RaiseException_EntityIsNotAFunction(int, var_t *);
void OSF_RaiseException_ReturnCalledOutsideFunction(int);
void OSF_RaiseException_IndexOutOfRange(int, int, array_t);
void OSF_RaiseException_ClassObjectIsNotAnIterable(int, class_t *);
void OSF_RaiseException_IterativeMustBeAClassObject(int);
void OSF_RaiseException_ClassObjectIsNotAnIterative(int, class_t *);
void OSF_RaiseException_DictIterationAllowsMax2SubstitutionVars(int, int);
void OSF_RaiseException_IterationCountMustBeAnInteger(int);
void OSF_RaiseException_ImportMustHaveAnAlias(int);
void OSF_RaiseException_ModuleHasNoConstructor(int, mod_t *);
void OSF_RaiseException_ModuleConstructorIsNotAFunction(int, var_t *);
void OSF_RaiseException_InlineAssignmentIsNotAllowedInModuleConstructors(int);
void OSF_RaiseException_StepCountMustBeAnInteger(int, expr_t);
void OSF_RaiseException_ToStepEntitiesMustBeAnInteger(int, expr_t, expr_t);
void OSF_RaiseException_ClassObjectIsNotCallable(int, class_t *);
void OSF_RaiseException_ClassObjectCannotBeIndexed(int, class_t *);
void OSF_RaiseException_ObjectHasNoMember(int, expr_t, char *);
void OSF_RaiseException_ModuleHasNoMember(int, mod_t *, char *);
void OSF_RaiseException_CannotOverloadDotOnEntity(int, expr_t);
void OSF_RaiseException_ArrayIndexMustBeAnInteger(int, expr_t);
void OSF_RaiseException_CannotUseUnaryMinusOnNonNumberEntity(int, expr_t);
void OSF_RaiseException_InvalidUsageOfOpPlus(int);
void OSF_RaiseException_InvalidNumberOfArgsPassed(int, int, int);
void OSF_RaiseException_FunctionWithVaArgsNeedsToHaveAnUndefinedVariable(int);
void OSF_RaiseException_UnknownEntitiesToCompare(int, expr_t, expr_t);
void OSF_RaiseException_EntityMustBeAString(int, expr_t);
void OSF_RaiseException_InvalidIterator(int);
void OSF_RaiseException_NonDefaultArgFollowsADefaultArg(int);
void OSF_RaiseException_NonDefaultArgFollowsVarArgs(int);
void OSF_RaiseException_InheritMustBeAClass(int);