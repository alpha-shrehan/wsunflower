#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Parser/str.h>

#include <Parser/psf_gen_inst_ast.h>

enum SFExceptionMessageCode
{
    EXCEPT_INDEX_OUT_OF_RANGE,
    EXCEPT_VARIABLE_DOES_NOT_EXIST,
    EXCEPT_CLASS_OBJ_NOT_AN_ITERABLE,
    EXCEPT_ENTITY_IS_NOT_A_FUNCTION,
    EXCEPT_ITERATIVE_MUST_BE_A_CLASS_OBJECT,
    EXCEPT_CLASS_OBJ_NOT_AN_ITERATIVE,
    EXCEPT_DICT_ITERATION_ALLOWS_MAX_2_SUBSTITUTION_VARS,
    EXCEPT_ITERATION_COUNT_MUST_BE_AN_INTEGER,
    EXCEPT_RETURN_CALLED_OUTSIZE_FUNCTION,
    EXCEPT_IMPORT_MUST_HAVE_AN_ALIAS,
    EXCEPT_SYNTAX_ERROR,
    EXCEPT_CODE_ASSERTION_ERROR,
    EXCEPT_CLASS_HAS_NO_CONSTRUCTOR,
    EXCEPT_MODULE_HAS_NO_CONSTRUCTOR,
    EXCEPT_MODULE_CONSTRUCTOR_IS_NOT_A_FUNCTION,
    EXCEPT_INLINE_ASSIGNMENT_IS_NOT_ALLOWED_IN_MODULE_CONSTRUCTOR,
    EXCEPT_STEP_COUNT_MUST_BE_AN_INTEGER,
    EXCEPT_TO_STEP_ENTITIES_MUST_BE_AN_INTEGER,
    EXCEPT_CLASS_OBJECT_IS_NOT_CALLABLE,
    EXCEPT_CLASS_OBJECT_CANNOT_BE_INDEXED,
    EXCEPT_OBJECT_HAS_NO_MEMBER,
    EXCEPT_MODULE_HAS_NO_MEMBER,
    EXCEPT_CANNOT_OVERLOAD_DOT_ON_ENTITY,
    EXCEPT_ARRAY_INDEX_MUST_BE_AN_INTEGER,
    EXCEPT_CANNOT_USE_UNARY_MINUS_ON_NON_NUMBER_ENTITY,
    EXCEPT_INVALID_USAGE_OF_OP_PLUS,
    EXCEPT_INVALID_NUMBER_OF_ARGS_PASSED,
    EXCEPT_FUNCTION_WITH_VARIABLE_ARGUMENTS_NEEDS_TO_HAVE_AN_UNDEFINED_VARIABLE,
    EXCEPT_UNKNOWN_ENTITIES_TO_COMPARE,
    EXCEPT_ENTITY_MUST_BE_A_STRING,
    EXCEPT_INVALID_ITERATOR
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
            int algn;
        } ce2;

        struct
        {
            int algn;
        } ce3;

        struct
        {
            int algn;
        } ce4;

        struct
        {
            int algn;
        } ce5;

        struct
        {
            int algn;
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
            int algn;
        } ce10;

        struct
        {
            int algn;
        } ce11;

        struct
        {
            int algn;
        } ce12;

        struct
        {
            int algn;
        } ce13;

        struct
        {
            int algn;
        } ce14;

        struct
        {
            int algn;
        } ce15;

        struct
        {
            int algn;
        } ce16;

        struct
        {
            int algn;
        } ce17;

        struct
        {
            int algn;
        } ce18;

        struct
        {
            int algn;
        } ce19;

        struct
        {
            int algn;
        } ce20;

        struct
        {
            int algn;
        } ce21;

        struct
        {
            int algn;
        } ce22;

        struct
        {
            int algn;
        } ce23;

        struct
        {
            int algn;
        } ce24;

        struct
        {
            int algn;
        } ce25;

        struct
        {
            int algn;
        } ce26;

        struct
        {
            int algn;
        } ce27;

        struct
        {
            int algn;
        } ce28;

        struct
        {
            int algn;
        } ce29;

        struct
        {
            int algn;
        } ce30;

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