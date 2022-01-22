#pragma once

#define BODY(X) _mod_body(X)
#define ARRAY(X) (*PSG_GetArray_Ptr((X)))
#define DICT(X) (*PSG_GetDict_Ptr((X)))

#include <assert.h>
#include <limits.h>
#include <stdarg.h>
#include <Object/osf_mem.h>
#include <Parser/psf_gen_ast.h>

enum ExprTypeEnum
{
    EXPR_TYPE_CONSTANT,
    EXPR_TYPE_VARIABLE,
    EXPR_TYPE_FUNCTION_CALL,
    EXPR_TYPE_FUNCTION,
    EXPR_TYPE_INLINE_ASSIGNMENT,
    EXPR_TYPE_LOGICAL_ARITHMETIC_OP,
    EXPR_TYPE_TO_STEP_CLAUSE,
    EXPR_TYPE_INDEX_OP,
    EXPR_TYPE_INLINE_FOR,
    EXPR_TYPE_UNARY_ARITHMETIC_OP,
    EXPR_TYPE_THEN_WHILE,
    EXPR_TYPE_INLINE_REPEAT,
    EXPR_TYPE_CLASS,
    EXPR_TYPE_MEMBER_ACCESS,
    EXPR_TYPE_MODULE,
    EXPR_TYPE_IN_CLAUSE
};

enum ConstantTypeEnum
{
    CONSTANT_TYPE_INT,
    CONSTANT_TYPE_FLOAT,
    CONSTANT_TYPE_STRING, // Strings in Sunflower start and end with single quote
    CONSTANT_TYPE_BOOL,
    CONSTANT_TYPE_DTYPE,
    CONSTANT_TYPE_ARRAY,
    CONSTANT_TYPE_CLASS_OBJECT,
    CONSTANT_TYPE_DICT
};

enum DataTypeEnum
{
    DATA_TYPE_NONE,
    DATA_TYPE_VOID
};

enum ExprLogicalOpEnum
{
    LOGICAL_OP_EQEQ,
    LOGICAL_OP_LEQ,
    LOGICAL_OP_GEQ,
    LOGICAL_OP_NEQ,
    LOGICAL_OP_LEEQ,
    LOGICAL_OP_GEEQ
};

/**
 * @brief These enums are arranged in order (increasing) of their
 * precedence in BODMAS (PEMDAS).
 * Operators who do not fall under this principle follow
 * Python's precedence ordering
 */
enum ExprArithmeticOpEnum
{
    UNARY_OP_SUB,
    UNARY_OP_ADD,
    UNARY_OP_MUL,
    UNARY_OP_DIV,
    UNARY_OP_MOD
};

enum ExprArithmeticOpOrderEnum
{
    ORDER_CONSTANT,
    ORDER_OPERATOR
};

struct _expr
{
    enum ExprTypeEnum type;
    struct _expr *next; // Backup for expressions that represent a symbol and need to have a value (example default function arguments)

    union
    {
        struct
        {
            enum ConstantTypeEnum constant_type;

            union
            {
                struct
                {
                    int value;
                } Int;

                struct
                {
                    double value;
                    int is_inf;
                } Float;

                struct
                {
                    char *value;
                } String;

                struct
                {
                    int value;
                } Bool;

                struct
                {
                    enum DataTypeEnum type;
                } DType;

                struct
                {
                    int index;
                } Array;

                struct
                {
                    int_tuple idx;
                } ClassObj;

                struct
                {
                    int index;
                } Dict;
            };
        } constant;

        struct
        {
            char *name;
        } variable;

        struct
        {
            int is_func_call;
            struct _expr *name;
            struct _expr *args;
            int arg_size;
        } function_call;

        struct
        {
            char *name;
            int index;
        } function_s;

        struct
        {
            struct _expr *lhs;
            struct _expr *rhs;
        } inline_assignment;

        struct
        {
            struct _expr *lhs;
            enum ExprLogicalOpEnum op;
            struct _expr *rhs;
        } logical_arith_op_expr;

        struct
        {
            struct _expr *lhs;
            struct _expr *rhs;
            struct _expr *_step;

        } to_step_clause;

        struct
        {
            struct _expr *entity;
            struct _expr *index;
        } index_op;

        struct
        {
            struct _expr *body;
            struct _expr *condition;
            void *vars; // Cast to (var_t *)
            int var_size;
        } inline_for;

        struct
        {
            struct _expr *sibs;
            int sibs_size;
            enum ExprArithmeticOpEnum *ops;
            int ops_size;
            enum ExprArithmeticOpOrderEnum *order;
            int order_count;

        } unary_arith_op_expr;

        struct
        {
            struct _expr *body;
            struct _expr *condition;
            void *withs; // Cast to (var_t *)
            int withs_size;
            void *assigns; // Case to (var_t *)
            int assigns_size;
        } then_while;

        struct
        {
            struct _expr *body;
            struct _expr *cond;
        } inline_repeat;

        struct
        {
            int index;
        } class_expr;

        struct
        {
            struct _expr *parent;
            char *child;
        } member_access;

        struct
        {
            int index;
        } module_s;

        struct
        {
            struct _expr *_lhs;
            struct _expr *_rhs;
        } in_clause;

    } v;
};

typedef struct _expr expr_t;

enum StatementTypeEnum
{
    STATEMENT_TYPE_EXPR,
    STATEMENT_TYPE_IF,
    STATEMENT_TYPE_ELIF,
    STATEMENT_TYPE_ELSE,
    STATEMENT_TYPE_WHILE,
    STATEMENT_TYPE_FOR,
    STATEMENT_TYPE_RETURN,
    STATEMENT_TYPE_BREAK,
    STATEMENT_TYPE_CONTINUE,
    STATEMENT_TYPE_VAR_DECL,
    STATEMENT_TYPE_FUNCTION_DECL,
    STATEMENT_TYPE_CLASS_DECL,
    STATEMENT_TYPE_IMPORT,
    STATEMENT_TYPE_COMMENT,
    STATEMENT_TYPE_VAR_REF,
    STATEMENT_TYPE_REPEAT,
    STATEMENT_TYPE_SWITCH,
    STATEMENT_TYPE_ASSERT
};

struct _conditional_struct
{
    expr_t *condition;
    struct _stmt *body;
    int body_size;

    struct _conditional_struct *elif_stmts; // Can be many
    int elif_stmts_count;

    /**
     * For .else_stmt
     * Only one entity.
     * condition will not be used.
     * elif_stmts will not be used.
     * elif_stmts_count will not be used
     * else_stmt will not be used
     */
    struct _conditional_struct *else_stmt;
};

struct __mod_child_varhold_s
{
    char *name;
    expr_t val;
};

typedef struct __mod_child_varhold_s var_t;

struct _stmt
{
    enum StatementTypeEnum type;

    union
    {
        struct
        {
            expr_t *expr;
        } expr;

        struct _conditional_struct if_stmt; // Because of nested _conditional_struct loops

        struct
        {
            expr_t *condition;
            struct _stmt *body;
            int body_size;
        } while_stmt;

        struct
        {
            var_t *vars;
            int var_size;
            expr_t *condition;
            struct _stmt *body;
            int body_size;
        } for_stmt;

        struct
        {
            expr_t *expr;
        } return_stmt;

        struct
        {
            char algn;
        } break_stmt;

        struct
        {
            char algn;
        } continue_stmt;

        struct
        {
            expr_t *name;
            expr_t *expr;
        } var_decl;

        struct
        {
            char *name;
            var_t *args;
            int arg_size;
            struct _stmt *body;
            int body_size;
            int takes_def_args;
            int takes_var_args;
        } function_decl;

        struct
        {
            char *name;
            struct _stmt *body;
            int body_size;
        } class_decl;

        struct
        {
            char *path;
            expr_t *args;
            int arg_count;
            char *alias;
        } import_s;

        struct
        {
            char *value;
        } comment;

        struct
        {
            char *name;
        } var_ref;

        struct
        {
            expr_t *condition;
            struct _stmt *body;
            int body_size;
        } repeat_stmt;

        struct
        {
            expr_t *condition;
            // For size, just do sizeof(sz_strct)
            struct
            {
                expr_t *condition;
                struct _stmt *body;
                int body_size;
                int is_case_in; // If 1, check for True, not ==
            } *cases, def_case, sz_strct; // condition is NULL for def_case
            int cases_count;
            
        } switch_case;

        struct
        {
            expr_t *condition;
            expr_t *message;
        } assert_stmt;

    } v;
};

typedef struct _stmt stmt_t;

enum ModuleTypeEnum
{
    MODULE_TYPE_FILE, // Main program and import modules
    MODULE_TYPE_FUNCTION,
    MODULE_TYPE_CLASS,
    MODULE_TYPE_INTERACTIVE // Shell
};

struct __mod_child_body_type_hold_s
{
    char *name;
    stmt_t *body;
    int body_size;
};

struct _mod
{
    enum ModuleTypeEnum type;

    struct __mod_child_varhold_s *var_holds;
    int var_holds_size;

    union
    {
        struct __mod_child_body_type_hold_s function_s,
            class_s,
            file_s;
    } v;

    expr_t *returns;
    struct _mod *parent;
    psf_byte_array_t *ast;
    void *class_objects; // Cast to class_t *
    int class_objects_count;
    char *path_prefix;
};

typedef struct _mod mod_t;

struct _fun
{
    char *name;
    int is_native;
    int arg_acceptance_count; // -1 for va_args, -2 for args with pre-def values

    union
    {
        struct
        {
            expr_t *args;
            int arg_size;
            expr_t *(*f)(mod_t *);
        } Native;

        struct
        {
            var_t *args;
            int arg_size;
            stmt_t *body;
            int body_size;
            // expr_t *returns;
        } Coded;

    } v;
};

typedef struct _fun fun_t;

struct _class
{
    char *name;
    int is_native;
    mod_t *mod;
    int object_count; // Acts as object counter for classes, and reference counter for class objects
    int killed;

    struct _class *objects;
};

typedef struct _class class_t;

struct _array
{
    expr_t *vals;
    int len;
    // struct _array *parent; // Only one instance
    int evaluated;
};

typedef struct _array array_t;

struct _dict
{
    expr_t *keys;
    expr_t *vals;
    int len;
    int evaluated;
};

typedef struct _dict dict_t;

/**
 * @brief This does not refer to data types in sunflower like None.
 * It refers to the general built in data types like int, string.
 * This stores variables for such data types.
 * Think of .__proto__ in JavaScript
 */
struct _dtype_proto_s {
    int type;
    var_t val;
};

typedef struct _dtype_proto_s DtypeProto_t;

/**
 * @brief Create a new module
 * @param mod_type Type of module
 * @param ast_ref AST reference dump
 * @return mod_t
 */
mod_t *SF_CreateModule(enum ModuleTypeEnum, psf_byte_array_t *);

/**
 * @brief Copy module and return it
 * @param mod Module
 * @return mod_t*
 */
mod_t *SF_ModuleCopy(mod_t *);

/**
 * @brief Destroy a module
 * @param mod Module
 */
void SF_DestroyModule(mod_t *);

/**
 * @brief Get module body respective to type of module
 * @param mod_ref Module reference
 * @return struct __mod_child_body_type_hold_s
 */
struct __mod_child_body_type_hold_s *_mod_body(mod_t *);

/**
 * @brief Compress bytecode to expression
 * @param arr Byte array
 * @return expr_t
 */
expr_t SF_FrameExpr_fromByte(psf_byte_array_t *);

/**
 * @brief Sunflower Instruction Tree from AST
 * @param mod Module reference
 */
void SF_FrameIT_fromAST(mod_t *);

/**
 * @brief Print type of statement
 * @param _en Statement enum
 */
void PSG_PrintStatementType(enum StatementTypeEnum);

/**
 * @brief Print type of expression
 * @param _en Expression enum
 */
void PSG_PrintExprType(enum ExprTypeEnum);

/**
 * @brief Convert Sunflower entity to C and return it
 * (does not tell you about the type of eneity)
 * @param en Expression enum
 * @return void* Ambiguous pointer
 */
void *SF_CCast_Entity(expr_t);

/**
 * @brief Add statement to module
 * @param mod Module
 * @param stmt Statement
 */
void PSG_AddStmt_toMod(mod_t *, stmt_t);

/**
 * @brief Get reference of variable that stores all functions of Sunflower
 * @return fun_t**
 */
fun_t **PSG_GetFunctions(void);

/**
 * @brief Get reference to variable that stores function holder's size
 * @return int *
 */
int *PSG_GetFunctionsSize(void);

/**
 * @brief Get reference of variable that stores all classes of Sunflower
 * @return class_t**
 */
class_t **PSG_GetClasses(void);

/**
 * @brief Get reference to variable that stores class holder's size
 * @return int*
 */
int *PSG_GetClassesSize(void);

/**
 * @brief Get reference of variable that stores all arrays of Sunflower
 * @return array_t**
 */
array_t **PSG_GetArrays(void);

/**
 * @brief Get reference to variable that stores array holder's size
 * @return int*
 */
int *PSG_GetArraysSize(void);

/**
 * @brief Get reference of variable that stores all dicts of Sunflower
 * @return dict_t** 
 */
dict_t **PSG_GetDicts(void);

/**
 * @brief Get reference to variable that stores dict holder's size
 * @return int* 
 */
int *PSG_GetDictSize(void);

/**
 * @brief Get reference to variable that stores module holder's size
 * @return int* 
 */
int *PSG_GetModuleHolderSize(void);

/**
 * @brief Get reference of variable that stores all imported modules of Sunflower
 * @return mod_t** 
 */
mod_t ***PSG_GetModules(void);

/**
 * @brief Add module to global module holder
 * @param mod Module
 * @return int 
 */
int PSG_AddModule(mod_t *);

/**
 * @brief Get module from index
 * @param idx Index
 * @return mod_t* 
 */
mod_t *PSG_GetModule(int);

/**
 * @brief Add array to global holder
 * @param arr Array
 * @return int 
 */
int PSG_AddArray(array_t);

/**
 * @brief Get array from index
 * @param idx Index
 * @return array_t 
 */
array_t *PSG_GetArray_Ptr(int);

/**
 * @brief Add dict to global holder
 * @param nd Dict
 * @return int 
 */
int PSG_AddDict(dict_t);

/**
 * @brief Get dict from index
 * @param idx Index
 * @return dict_t* 
 */

dict_t *PSG_GetDict_Ptr(int);

/**
 * @brief Environment Initializer for Sunflower PSG
 */
void _PSG_EnvInit(void);

/**
 * @brief Add function to function holder
 * @param _fun Function
 * @return int
 */
int PSG_AddFunction(fun_t);

/**
 * @brief Add class to class holder
 * @param _class Class
 * @return int
 */
int PSG_AddClass(class_t);

/**
 * @brief Add class instance to module
 * @param _class Class
 * @param mod Module
 * @return int
 */
int PSG_AddClassObject_toModule(class_t, mod_t *);

/**
 * @brief Adds class to object counter of global classes storage stack
 * @param _class Class
 * @param idx Class index
 * @return int_tuple
 */
int_tuple PSG_AddClassObject_toClass(class_t, int);

/**
 * @brief Get current tab scope
 * @param arr Array
 * @param st Start index
 * @return int
 */
int _PSF_GetTabspace(psf_byte_array_t *, int);

/**
 * @brief Get tabbed body of a loop
 * @param arr Array
 * @param st Start index
 * @param tab Tabspace
 * @return psf_byte_array_t*
 */
psf_byte_array_t *_PSF_GetBody(psf_byte_array_t *, int, int);

/**
 * @brief Check if an entity is true
 * @param ent Entity
 * @return int
 */
int _PSF_EntityIsTrue(expr_t);

/**
 * @brief Remove SF voids from an expr_t array
 * @param arr Array
 * @param size Size of array
 * @param sz_ptr Result size pointer
 * NOTE: Store the reference and clear it later
 * @return expr_t*
 */
expr_t *_PSF_RemoveVoidsFromExprArray(expr_t *, int, int *);

/**
 * @brief Construct for loop AST from bytes
 * @param mod Module
 * @param idx Start index
 * @param body_sz_ptr body AST size pointer
 * @return stmt_t
 */
stmt_t _PSF_ConstructForLoopStmt(mod_t *, int, int *);

/**
 * @brief Construct inline for loop AST from bytes
 * @param arr Array
 * @param idx Index where for loop was encountered
 * @return expr_t
 */
expr_t _PSF_ConstructInlineForExpr(psf_byte_array_t *, int);

/**
 * @brief Construct variable from AST
 * Types: (var), var, ((var)), ((var) = (expr)), var = expr.
 * Used in for loops
 * @param arr Array
 * @return var_t
 */
var_t _PSF_GenerateVarFromByte(psf_byte_array_t *);

/**
 * @brief Construct inline while..then loop AST from bytes
 * @param arr Array
 * @param idx Index where while keyword was encountered
 * @return expr_t
 */
expr_t _PSF_ConstructWhileThenExpr(psf_byte_array_t *, int);

/**
 * @brief Construct repeat loop from bytes
 * @param mod Module
 * @param idx Start index
 * @param bd_sz_ptr Body size pointer
 * @return stmt_t
 */
stmt_t _PSF_ConstructRepeatLoopStmt(mod_t *, int, int *);

/**
 * @brief Construct repeat expression
 * @param arr Array
 * @param idx Start index
 * @return expr_t
 */
expr_t _PSF_ConstructRepeatExpr(psf_byte_array_t *, int);

/**
 * @brief Construct function declaration AST from bytes
 * @param arr Array
 * @param idx Start index
 * @param bd_sz_ptr body AST size pointer
 * @return stmt_t
 */
stmt_t _PSF_ConstructFunctionStmt(psf_byte_array_t *, int, int *);

/**
 * @brief Create class from bytes
 * @param arr Array
 * @param idx Start index
 * @param res_loc_ptr Result index (to be directly assigned to i)
 * @return stmt_t
 */
stmt_t _PSF_ConstructClassStmt(psf_byte_array_t *, int, size_t *);

/**
 * @brief Get reference to the Dtype Proto Holder object
 * @return DtypeProto_t** 
 */

DtypeProto_t **GetDtypeProtoHolder(void);

/**
 * @brief Get reference to the Dtype Proto Holder object's size
 * @return int* 
 */
int *GetDtypeProtoHolderSize(void);

/**
 * @brief Add a new prototype value
 * @param type Type it refers to
 * @param name Symbol name
 * @param val Symbol value
 * @return int
 */
int AddDtypePrototype(int, char *, expr_t);

/**
 * @brief Get prototype variable from symbol and type it belongs to
 * @param type Type
 * @param name Symbol
 * @return var_t* (pointer because it can be NULL)
 */
var_t *GetDtypePrototype_fromSymbolAndType(int, char *);

/**
 * @brief Construct import line from bytecode
 * @param mod Module
 * @param idx Start index
 * @return stmt_t 
 */
stmt_t _PSF_ConstructImportLine(mod_t *, int);

/**
 * @brief Construct switch statement from bytecode
 * @param arr Array
 * @param idx Start index
 * @param end_ptr End index pointer
 * @return stmt_t 
 */
stmt_t _PSF_ConstructSwitchStmt(psf_byte_array_t *, int, size_t *);

/**
 * @brief Get valid import path
 * @param name_path Path
 * @param extra_paths_count Extra explicit paths
 * @return char* 
 */
char *_PSF_GetValidImportPath(char *, int, ...);

/**
 * @brief Construct dict from code
 * @param arr Array
 * @param st Start index
 * @param ed_ptr End pointer
 * @return expr_t* 
 */
expr_t _PSF_ConstructDict_fromByte(psf_byte_array_t *, int, int *);

/**
 * @brief Construct individual key value pair from byte
 * @param arr Array
 * @param res Reference to array of size 2
 */
void _PSF_ConstructIndividualKeyVal_forDict_fromByte(psf_byte_array_t *, expr_t ***);

/**
 * @brief Execute logical arithmetic operations
 * @param _lhs Left hand operand
 * @param op_ty Operand
 * @param _rhs Right hand operand
 * @return expr_t 
 */
expr_t _IPSF_ExecLogicalArithmetic(expr_t, int, expr_t);