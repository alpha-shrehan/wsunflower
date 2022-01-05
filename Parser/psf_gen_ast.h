#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Object/osf_mem.h>

#define SF_FMT(X) X
#define SF_STRING(X) _PSF_TrimSFStrImp(X)

enum psf_gen_ast_nval_type_e
{
    AST_NVAL_TYPE_DATA_TYPE,
    AST_NVAL_TYPE_INT,
    AST_NVAL_TYPE_FLOAT,
    AST_NVAL_TYPE_STRING,
    AST_NVAL_TYPE_IDENTIFIER,
    AST_NVAL_TYPE_NEWLINE,
    AST_NVAL_TYPE_COMMENT,
    AST_NVAL_TYPE_TABSPACE,
    AST_NVAL_TYPE_OPERATOR,
    AST_NVAL_TYPE_BOOL
};

struct psf_gen_ast_nval_s
{
    enum psf_gen_ast_nval_type_e nval_type;

    union
    {
        struct
        {
            char *val;
        } DataType;

        struct
        {
            int val;
        } Int;

        struct
        {
            double val;
        } Float;

        struct
        {
            char *val;
        } String;

        struct
        {
            int is_token; // 0 = identifier, 1 = token
            char *val;
        } Identifier;

        struct
        {
            char x; // Ignore this value, only used for alignment (minimum size of 1 byte)
        } Newline;

        struct
        {
            char *cmt;
        } Comment;

        struct
        {
            int len;
        } Tabspace;

        struct
        {
            char *val;
        } Operator;

        struct
        {
            int val;
        } Bool;

    } v;
};

typedef struct psf_gen_ast_nval_s psf_byte_t;
typedef psf_byte_t *psf_byte_ptr_t;
typedef struct
{
    psf_byte_t *nodes;
    int size;
} psf_byte_array_t;


typedef struct
{
    int x1;
    int x2;
} int_tuple;

void PSF_AST_ByteArray_AddNode(psf_byte_array_t *, psf_byte_t);

/**
 * @brief Generate AST from string
 * @param src Source string
 * @return psf_byte_array_t* 
 */
psf_byte_array_t *PSF_AST_fromString(const char *);

/**
 * @brief Print AST to stdout
 * @param ast AST reference
 */
void PSF_AST_print(psf_byte_array_t *);

/**
 * @brief Construct operator from string
 * @param op1 First operand
 * @param op2 Second operand
 * @param op3 Third operand
 * @note Max length of operator = 3
 * @return char* 
 */
char *_PSF_Construct_Operator_fromString(char, char, char);

/**
 * @brief Main function to generate AST from string (normal)
 * @param src Source of string
 * @return psf_byte_array_t* 
 */
psf_byte_array_t *_PSF_New_AST_FromString(const char *);

/**
 * @brief New byte array
 * @return psf_byte_array_t* 
 */
psf_byte_array_t *_PSF_newByteArray(void);

/**
 * @brief Remove the first and last char (') from SF string
 * @param _str String
 * @return char* 
 */
char *_PSF_TrimSFStrImp(char *);