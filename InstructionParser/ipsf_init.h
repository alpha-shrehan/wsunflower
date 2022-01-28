#pragma once

#include <Parser/psf_gen_ast.h>
#include <Parser/psf_gen_inst_ast.h>

#include <Object/osf_except.h>

#define IPSF_OK -1
#define IPSF_NOT_OK_CHECK_EXPR_LOG -2
#define IPSF_ERR_VAR_NOT_FOUND 100
#define IPSF_ERR_EXCEPTION 101

enum IPSF_ErrorTypes
{
    ERROR_SYNTAX,
    ERROR_REFERENCE,
};

/**
 * @brief Initialize the instruction parser.
 * @param mod Module
 * @param err Error code pointer (can be NULL)
 * @return expr_t* 
 */
expr_t *IPSF_ExecIT_fromMod(mod_t *, int *);

/**
 * @brief Evaluates statements of type STATEMENT_TYPE_EXPR
 * @param mod Module
 * @param stmt Statement
 * @param err Error variable (can be NULL)
 * @return expr_t* 
 */
expr_t *IPSF_ExecExprStatement_fromMod(mod_t *, stmt_t, int *);

/**
 * @brief Add variable to heap
 * @param mod Module
 * @param stmt Statement. Type must be STATEMENT_TYPE_VAR_DECL
 * @param err Error code pointer (can be NULL)
 * @return struct __mod_child_varhold_s* 
 */
struct __mod_child_varhold_s *IPSF_ExecVarDecl_fromStmt(mod_t *, stmt_t, int *);

/**
 * @brief Get variable from heap
 * @param mod Module
 * @param name Name
 * @param err Error code pointer (can be NULL)
 * @return struct __mod_child_varhold_s* 
 */
struct __mod_child_varhold_s *IPSF_GetVar_fromMod(mod_t *, const char *, int *);

/**
 * @brief Reduce a raw expression (like EXPR_TYPE_VARIABLE) to constant.
 * Internally calls IPSF_ExecExprStatement_fromMod()
 * @param mod Module
 * @param expr Expression
 * @return expr_t 
 */
expr_t IPSF_ReduceExpr_toConstant(mod_t *, expr_t);

/**
 * @brief Checks if a data type is void
 * @param expr Expression to check
 * @return int 
 */
int _IPSF_IsDataType_Void(expr_t);

/**
 * @brief Checks if a data type is None
 * @param expr Expression to check
 * @return int 
 */
int _IPSF_IsDataType_None(expr_t);

/**
 * @brief Default stdout/string representation for an entity
 * @param expr Expression
 * @param recur Is recursive (string with quotes)
 * @return char* 
 */
char *_IPSF_ObjectRepr(expr_t, int);

/**
 * @brief Add a variable to module
 * @param mod Module
 * @param name Name of variable
 * @param val Value of variable
 * @return var_t* 
 */
var_t *_IPSF_AddVar_toModule(mod_t *, char *, expr_t);

/**
 * @brief Strictly add variable to module, 
 * Regardless of it's presence in parents
 * @param mod Module
 * @param name Name of variable
 * @param val Value of variable
 * @return var_t* 
 */
var_t *_IPSF_AddVar_toModule_strict(mod_t *, char *, expr_t);

/**
 * @brief Execute arithmetic operation from expression which is suitable. 
 * @param mod Module
 * @param expr Expression
 * @return expr_t 
 */
expr_t _IPSF_ExecUnaryArithmetic(mod_t *, expr_t *);

/**
 * @brief Create a dynamic pointer and return it
 * @param stmt Object
 * @return stmt_t* 
 */
__attribute__ ((__deprecated__)) stmt_t *_SFPTR_StmtPtr(stmt_t);

/**
 * @brief Call a function and return it's result
 * @param fun Function
 * @param args Arguments
 * @param arg_size Arguments size
 * @param mod Module reference
 * @return expr_t* 
 */
expr_t *_IPSF_CallFunction(fun_t, expr_t *, int, mod_t *);

/**
 * @brief Safe delete module, with important references left undisturbed.
 * Entities deleted: variables, class objects
 * @param mod Module
 */
void SF_Module_safeDelete(mod_t *);

/**
 * @brief Destroy classes
 * @param mod Module
 */
void _IPSF_DestClasses(mod_t *);

/**
 * @brief Get class from tuple config
 * @param tup Tuple
 * @return class_t* 
 */
class_t *_IPSF_GetClass_fromIntTuple(int_tuple);

/**
 * @brief Check if entity is present in another entity
 * @param lhs LHS
 * @param rhs RHS
 * @param mod Module
 * @return expr_t Sunflower Boolean
 */
expr_t _IPSF_Entity_IsIn_Entity(expr_t, expr_t, mod_t *);

/**
 * @brief Get directory from file path
 * @param path Path
 * @return char* 
 */
char *_IPSF_GetDir_FromFilePath(char *);