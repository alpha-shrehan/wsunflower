#include "psf_gen_inst_ast.h"

static fun_t *PSG_Functions;
static int PSG_Functions_Size;
static class_t *PSG_Classes;
static int PSG_Classes_Size;

mod_t *SF_CreateModule(enum ModuleTypeEnum mod_type, psf_byte_array_t *ast_ref)
{
    mod_t *new_mod = (mod_t *)OSF_Malloc(sizeof(mod_t));

    assert(new_mod != NULL);

    new_mod->parent = NULL;
    new_mod->type = mod_type;
    new_mod->var_holds = (struct __mod_child_varhold_s *)OSF_Malloc(sizeof(struct __mod_child_varhold_s));
    new_mod->var_holds_size = 0;
    new_mod->ast = ast_ref;
    new_mod->returns = NULL;
    new_mod->class_objects = NULL;
    new_mod->class_objects_count = 0;

    switch (mod_type)
    {
    case MODULE_TYPE_FILE:
    {
        new_mod->v.file_s.body = (struct _stmt *)OSF_Malloc(((new_mod->v.file_s.body_size = 0) + 1) * sizeof(struct _stmt));
        new_mod->v.file_s.name = NULL;
    }
    break;
    case MODULE_TYPE_FUNCTION:
    {
        new_mod->v.function_s.body = (struct _stmt *)OSF_Malloc(((new_mod->v.function_s.body_size = 0) + 1) * sizeof(struct _stmt));
        new_mod->v.function_s.name = NULL;
    }
    break;
    case MODULE_TYPE_CLASS:
    {
        new_mod->v.class_s.body = (struct _stmt *)OSF_Malloc(((new_mod->v.class_s.body_size = 0) + 1) * sizeof(struct _stmt));
        new_mod->v.class_s.name = NULL;
    }
    break;
    default:
        break;
    }

    return new_mod;
}

mod_t *SF_ModuleCopy(mod_t *mod)
{
    mod_t *res = SF_CreateModule(mod->type, mod->ast);
    OSF_Free(BODY(res)->body);

    res->var_holds_size = mod->var_holds_size;
    res->parent = mod->parent;
    res->returns = mod->returns;
    BODY(res)->body = BODY(mod)->body;
    BODY(res)->body_size = BODY(mod)->body_size;
    BODY(res)->name = BODY(mod)->name;

    if (mod->var_holds_size)
    {
        OSF_Free(res->var_holds);
        res->var_holds = OSF_Malloc(mod->var_holds_size * sizeof(var_t));
        for (size_t i = 0; i < res->var_holds_size; i++)
            res->var_holds[i] = mod->var_holds[i];
    }

    return res;
}

struct __mod_child_body_type_hold_s *_mod_body(mod_t *mod_ref)
{
    switch (mod_ref->type)
    {
    case MODULE_TYPE_FILE:
        return &(mod_ref->v.file_s);
        break;
    case MODULE_TYPE_CLASS:
        return &(mod_ref->v.class_s);
        break;
    case MODULE_TYPE_FUNCTION:
        return &(mod_ref->v.function_s);
        break;
    default:
        break;
    }

    return NULL;
}

expr_t SF_FrameExpr_fromByte(psf_byte_array_t *arr)
{
    expr_t ret;
    int ret_now = 0;

    for (int i = arr->size - 1; i >= 0; i--)
    {
        psf_byte_t curr = arr->nodes[i];
        int doBreak = 0;

        switch (curr.nval_type)
        {
        case AST_NVAL_TYPE_IDENTIFIER:
        {
            const char *tok = curr.v.Identifier.val;

            if (curr.v.Identifier.is_token)
            {
                if (!strcmp(tok, "for"))
                {
                    // Inline `for'
                    expr_t _res = _PSF_ConstructInlineForExpr(arr, i);
                    ret = _res;
                    doBreak = 1;
                    ret_now = 1;
                    break;
                }
                else if (!strcmp(tok, "while"))
                {
                    // Inline `while'
                    expr_t _res = _PSF_ConstructWhileThenExpr(arr, i);
                    ret = _res;
                    doBreak = 1;
                    ret_now = 1;
                    break;
                }
                else if (!strcmp(tok, "repeat"))
                {
                    // Inline `repeat'
                    expr_t _res = _PSF_ConstructRepeatExpr(arr, i);
                    ret = _res;
                    doBreak = 1;
                    ret_now = 1;
                    break;
                }
            }
        }
        break;

        default:
            break;
        }

        if (doBreak)
            break;
    }

    if (ret_now)
        return ret;

    for (int i = 0; i < arr->size; i++)
    {
        psf_byte_t curr = arr->nodes[i];
        int doBreak = 0;

        switch (curr.nval_type)
        {
        case AST_NVAL_TYPE_INT:
        case AST_NVAL_TYPE_FLOAT:
        case AST_NVAL_TYPE_DATA_TYPE:
        case AST_NVAL_TYPE_STRING:
        case AST_NVAL_TYPE_BOOL:
        {
            ret.type = EXPR_TYPE_CONSTANT;

            switch (curr.nval_type)
            {
            case AST_NVAL_TYPE_INT:
                ret.v.constant.constant_type = CONSTANT_TYPE_INT;
                ret.v.constant.Int.value = curr.v.Int.val;
                break;
            case AST_NVAL_TYPE_FLOAT:
                ret.v.constant.Float.is_inf = 0;
                ret.v.constant.constant_type = CONSTANT_TYPE_FLOAT;
                ret.v.constant.Float.value = curr.v.Float.val;
                break;
            case AST_NVAL_TYPE_DATA_TYPE:
            {
                ret.v.constant.constant_type = CONSTANT_TYPE_DTYPE;
                if (!strcmp(curr.v.DataType.val, "None"))
                    ret.v.constant.DType.type = DATA_TYPE_NONE;
            }
            break;
            case AST_NVAL_TYPE_STRING:
                ret.v.constant.constant_type = CONSTANT_TYPE_STRING;
                ret.v.constant.String.value = curr.v.String.val;
                break;
            case AST_NVAL_TYPE_BOOL:
                ret.v.constant.constant_type = CONSTANT_TYPE_BOOL;
                ret.v.constant.Bool.value = curr.v.Bool.val;
                break;

            default:
                break;
            }
        }
        break;
        case AST_NVAL_TYPE_IDENTIFIER:
        {
            // assert(!curr.v.Identifier.is_token && SF_FMT("Error: Cannot create expression out of a token."));

            if (!curr.v.Identifier.is_token)
            {
                ret.type = EXPR_TYPE_VARIABLE;
                ret.v.variable.name = strdup(curr.v.Identifier.val);
            }
            else
            {
                const char *tok = (const char *)curr.v.Identifier.val;

                if (!strcmp(tok, "to"))
                {
                    expr_t *c_lhs = OSF_Malloc(sizeof(expr_t));
                    *c_lhs = ret;

                    psf_byte_array_t *c_rhs = _PSF_newByteArray();
                    int gb = 0, j = i + 1;

                    while (j < arr->size)
                    {
                        psf_byte_t cv = arr->nodes[j];

                        if (cv.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(cv.v.Operator.val, "(") ||
                                !strcmp(cv.v.Operator.val, "[") ||
                                !strcmp(cv.v.Operator.val, "{"))
                                gb++;
                            else if (!strcmp(cv.v.Operator.val, ")") ||
                                     !strcmp(cv.v.Operator.val, "]") ||
                                     !strcmp(cv.v.Operator.val, "}"))
                                gb--;
                        }

                        if (cv.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
                            !strcmp(cv.v.Identifier.val, "step") &&
                            cv.v.Identifier.is_token &&
                            !gb)
                        {
                            break;
                        }

                        PSF_AST_ByteArray_AddNode(c_rhs, cv);
                        j++;
                    }

                    expr_t c_rhs_v = SF_FrameExpr_fromByte(c_rhs);

                    ret.type = EXPR_TYPE_TO_STEP_CLAUSE;
                    ret.v.to_step_clause.lhs = c_lhs;
                    ret.v.to_step_clause.rhs = OSF_Malloc(sizeof(expr_t));
                    *(ret.v.to_step_clause.rhs) = c_rhs_v;
                    ret.v.to_step_clause._step = OSF_Malloc(sizeof(expr_t));
                    *(ret.v.to_step_clause._step) = (expr_t){
                        .type = EXPR_TYPE_CONSTANT,
                        .v = {
                            .constant = {
                                .constant_type = CONSTANT_TYPE_INT,
                                .Int = {
                                    .value = 1}}}};

                    i = j - 1;

                    OSF_Free(c_rhs->nodes);
                    OSF_Free(c_rhs);
                }
                else if (!strcmp(tok, "step"))
                {
                    assert(
                        ret.type == EXPR_TYPE_TO_STEP_CLAUSE &&
                        SF_FMT("Error: stray `step'"));

                    psf_byte_array_t *_val = _PSF_newByteArray();

                    for (size_t j = i + 1; j < arr->size; j++)
                        PSF_AST_ByteArray_AddNode(_val, arr->nodes[j]);

                    expr_t _vred = SF_FrameExpr_fromByte(_val);

                    OSF_Free(_val->nodes);
                    OSF_Free(_val);

                    ret.v.to_step_clause._step = OSF_Malloc(sizeof(expr_t));
                    *(ret.v.to_step_clause._step) = _vred;

                    doBreak = 1;
                    break;
                }
            }
        }
        break;
        case AST_NVAL_TYPE_OPERATOR:
        {
            const char *op = curr.v.Operator.val;

            if (!strcmp(op, "="))
            {
                ret.type = EXPR_TYPE_INLINE_ASSIGNMENT;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.inline_assignment.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.inline_assignment.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.inline_assignment.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.inline_assignment.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, "=="))
            {
                ret.type = EXPR_TYPE_LOGICAL_ARITHMETIC_OP;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                ret.v.logical_arith_op_expr.op = LOGICAL_OP_EQEQ;

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, "!="))
            {
                ret.type = EXPR_TYPE_LOGICAL_ARITHMETIC_OP;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                ret.v.logical_arith_op_expr.op = LOGICAL_OP_NEQ;

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, "<"))
            {
                ret.type = EXPR_TYPE_LOGICAL_ARITHMETIC_OP;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                ret.v.logical_arith_op_expr.op = LOGICAL_OP_LEQ;

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, ">"))
            {
                ret.type = EXPR_TYPE_LOGICAL_ARITHMETIC_OP;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                ret.v.logical_arith_op_expr.op = LOGICAL_OP_GEQ;

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, "<="))
            {
                ret.type = EXPR_TYPE_LOGICAL_ARITHMETIC_OP;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                ret.v.logical_arith_op_expr.op = LOGICAL_OP_LEEQ;

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, ">="))
            {
                ret.type = EXPR_TYPE_LOGICAL_ARITHMETIC_OP;
                psf_byte_array_t *lhs_bt_arr = _PSF_newByteArray();
                lhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc(i * sizeof(psf_byte_t));
                lhs_bt_arr->size = i;
                memcpy(lhs_bt_arr->nodes, arr->nodes, i * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.lhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.lhs) = SF_FrameExpr_fromByte(lhs_bt_arr);

                ret.v.logical_arith_op_expr.op = LOGICAL_OP_GEEQ;

                psf_byte_array_t *rhs_bt_arr = _PSF_newByteArray();
                rhs_bt_arr->nodes = (psf_byte_t *)OSF_Malloc((arr->size - i) * sizeof(psf_byte_t));
                rhs_bt_arr->size = arr->size - i;
                memcpy(rhs_bt_arr->nodes, arr->nodes + i + 1, (arr->size - i) * sizeof(psf_byte_t));
                ret.v.logical_arith_op_expr.rhs = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *(ret.v.logical_arith_op_expr.rhs) = SF_FrameExpr_fromByte(rhs_bt_arr);
                doBreak = 1;

                OSF_Free(lhs_bt_arr->nodes);
                OSF_Free(lhs_bt_arr);
                OSF_Free(rhs_bt_arr->nodes);
                OSF_Free(rhs_bt_arr);

                break;
            }
            else if (!strcmp(op, "+") ||
                     !strcmp(op, "-") ||
                     !strcmp(op, "*") ||
                     !strcmp(op, "/") ||
                     !strcmp(op, "%"))
            {
                expr_t *sibs = OSF_Malloc(sizeof(expr_t));
                int sib_size = 0;

                enum ExprArithmeticOpOrderEnum *order = OSF_Malloc(sizeof(enum ExprArithmeticOpOrderEnum));
                int order_count = 0;

                enum ExprArithmeticOpEnum *ops = OSF_Malloc(sizeof(enum ExprArithmeticOpEnum));
                int op_size = 0;

                if (i)
                {
                    *(sibs + sib_size++) = ret;
                    *(order + order_count++) = ORDER_CONSTANT;
                }

                if (!strcmp(op, "+"))
                    *(ops + op_size++) = UNARY_OP_ADD;
                else if (!strcmp(op, "-"))
                    *(ops + op_size++) = UNARY_OP_SUB;
                else if (!strcmp(op, "*"))
                    *(ops + op_size++) = UNARY_OP_MUL;
                else if (!strcmp(op, "/"))
                    *(ops + op_size++) = UNARY_OP_DIV;
                else if (!strcmp(op, "%"))
                    *(ops + op_size++) = UNARY_OP_MOD;

                if (order_count)
                    order = OSF_Realloc(order, (order_count + 1) * sizeof(enum ExprArithmeticOpOrderEnum));

                order[order_count++] = ORDER_OPERATOR;

                psf_byte_array_t *_idx_arr = _PSF_newByteArray();

                int gb = 0;
                i++; // Eat '<op>'

                while (i < arr->size)
                {
                    psf_byte_t c = arr->nodes[i];

                    if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                    {
                        if (!strcmp(c.v.Operator.val, "(") ||
                            !strcmp(c.v.Operator.val, "{") ||
                            !strcmp(c.v.Operator.val, "["))
                            gb++;
                        else if (
                            !strcmp(c.v.Operator.val, ")") ||
                            !strcmp(c.v.Operator.val, "}") ||
                            !strcmp(c.v.Operator.val, "]"))
                            gb--;

                        if ((!strcmp(c.v.Operator.val, "+") ||
                             !strcmp(c.v.Operator.val, "-") ||
                             !strcmp(c.v.Operator.val, "*") ||
                             !strcmp(c.v.Operator.val, "/") ||
                             !strcmp(c.v.Operator.val, "%")) &&
                            !gb)
                        {
                            if (_idx_arr->size)
                            {
                                if (sib_size)
                                    sibs = OSF_Realloc(sibs, (sib_size + 1) * sizeof(expr_t));

                                sibs[sib_size++] = SF_FrameExpr_fromByte(_idx_arr);

                                if (order_count)
                                    order = OSF_Realloc(order, (order_count + 1) * sizeof(enum ExprArithmeticOpOrderEnum));

                                order[order_count++] = ORDER_CONSTANT;

                                OSF_Free(_idx_arr->nodes);
                                OSF_Free(_idx_arr);
                                _idx_arr = _PSF_newByteArray();
                            }

                            enum ExprArithmeticOpEnum r;

                            if (!strcmp(c.v.Operator.val, "+"))
                                r = UNARY_OP_ADD;
                            else if (!strcmp(c.v.Operator.val, "-"))
                                r = UNARY_OP_SUB;
                            else if (!strcmp(c.v.Operator.val, "*"))
                                r = UNARY_OP_MUL;
                            else if (!strcmp(c.v.Operator.val, "/"))
                                r = UNARY_OP_DIV;
                            else if (!strcmp(c.v.Operator.val, "%"))
                                r = UNARY_OP_MOD;

                            if (op_size)
                                ops = OSF_Realloc(ops, (op_size + 1) * sizeof(enum ExprArithmeticOpEnum));

                            ops[op_size++] = r;

                            if (order_count)
                                order = OSF_Realloc(order, (order_count + 1) * sizeof(enum ExprArithmeticOpOrderEnum));

                            order[order_count++] = ORDER_OPERATOR;
                            i++;
                            continue;
                        }
                    }

                    PSF_AST_ByteArray_AddNode(_idx_arr, c);

                    i++;
                }

                if (_idx_arr->size)
                {
                    if (sib_size)
                        sibs = OSF_Realloc(sibs, (sib_size + 1) * sizeof(expr_t));

                    sibs[sib_size++] = SF_FrameExpr_fromByte(_idx_arr);

                    if (order_count)
                        order = OSF_Realloc(order, (order_count + 1) * sizeof(enum ExprArithmeticOpOrderEnum));

                    order[order_count++] = ORDER_CONSTANT;

                    OSF_Free(_idx_arr->nodes);
                    OSF_Free(_idx_arr);
                }

                ret.type = EXPR_TYPE_UNARY_ARITHMETIC_OP;
                ret.v.unary_arith_op_expr.ops = ops;
                ret.v.unary_arith_op_expr.ops_size = op_size;
                ret.v.unary_arith_op_expr.sibs = sibs;
                ret.v.unary_arith_op_expr.sibs_size = sib_size;
                ret.v.unary_arith_op_expr.order = order;
                ret.v.unary_arith_op_expr.order_count = order_count;

                OSF_Free(_idx_arr->nodes);
                OSF_Free(_idx_arr);
                doBreak = 1;
                break;
            }
            else if (!strcmp(op, "("))
            {
                mod_t *temp_mod = SF_CreateModule(MODULE_TYPE_FILE, arr);
                SF_FrameIT_fromAST(temp_mod);

                ret = *((*(BODY(temp_mod)->body)).v.expr.expr);

                OSF_Free(BODY(temp_mod)->body);
                OSF_Free(temp_mod->var_holds);
                OSF_Free(temp_mod);
                doBreak = 1;
                break;
            }
            else if (!strcmp(op, "["))
            {
                if (i)
                {
                    expr_t ret_cpy = ret;
                    ret.type = EXPR_TYPE_INDEX_OP;
                    ret.v.index_op.entity = OSF_Malloc(sizeof(expr_t));
                    *(ret.v.index_op.entity) = ret_cpy;

                    psf_byte_array_t *index_ar = _PSF_newByteArray();
                    int gb = 0;
                    i++;

                    while (i < arr->size)
                    {
                        psf_byte_t c = arr->nodes[i];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "]") && !gb)
                                break;
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (
                                !strcmp(c.v.Operator.val, ")") ||
                                !strcmp(c.v.Operator.val, "}") ||
                                !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        PSF_AST_ByteArray_AddNode(index_ar, c);
                        i++;
                    }

                    expr_t index_ar_expr = SF_FrameExpr_fromByte(index_ar);

                    ret.v.index_op.index = OSF_Malloc(sizeof(expr_t));
                    *(ret.v.index_op.index) = index_ar_expr;

                    OSF_Free(index_ar->nodes);
                    OSF_Free(index_ar);
                }
                else
                {
                    ret.type = EXPR_TYPE_CONSTANT;
                    ret.v.constant.constant_type = CONSTANT_TYPE_ARRAY;
                    ret.v.constant.Array.len = 0;
                    ret.v.constant.Array.vals = OSF_Malloc(sizeof(expr_t));

                    psf_byte_array_t *_idx_arr = _PSF_newByteArray();

                    i++; // Eat '['
                    int gb = 0;

                    while (i < arr->size)
                    {
                        psf_byte_t c = arr->nodes[i];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "]") &&
                                !gb)
                            {
                                expr_t _fr_vi = SF_FrameExpr_fromByte(_idx_arr);
                                if (ret.v.constant.Array.len)
                                    ret.v.constant.Array.vals = OSF_Realloc(
                                        ret.v.constant.Array.vals,
                                        (ret.v.constant.Array.len + 1) *
                                            sizeof(expr_t));

                                ret.v.constant.Array.vals[ret.v.constant.Array.len++] = _fr_vi;

                                OSF_Free(_idx_arr->nodes);
                                OSF_Free(_idx_arr);
                                break;
                            }
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (
                                !strcmp(c.v.Operator.val, ")") ||
                                !strcmp(c.v.Operator.val, "}") ||
                                !strcmp(c.v.Operator.val, "]"))
                                gb--;

                            if (!strcmp(c.v.Operator.val, ",") &&
                                !gb)
                            {
                                expr_t _fr_vi = SF_FrameExpr_fromByte(_idx_arr);
                                if (ret.v.constant.Array.len)
                                    ret.v.constant.Array.vals = OSF_Realloc(
                                        ret.v.constant.Array.vals,
                                        (ret.v.constant.Array.len + 1) *
                                            sizeof(expr_t));

                                ret.v.constant.Array.vals[ret.v.constant.Array.len++] = _fr_vi;

                                OSF_Free(_idx_arr->nodes);
                                OSF_Free(_idx_arr);
                                _idx_arr = _PSF_newByteArray();
                                i++;
                                continue;
                            }
                        }

                        PSF_AST_ByteArray_AddNode(_idx_arr, c);

                        i++;
                    }
                }
            }
            else if (!strcmp(op, "."))
            {
                expr_t ret_cpy = ret;
                ret.type = EXPR_TYPE_MEMBER_ACCESS;
                ret.v.member_access.parent = OSF_Malloc(sizeof(expr_t));
                *(ret.v.member_access.parent) = ret_cpy;
                assert(
                    arr->nodes[i + 1].nval_type == AST_NVAL_TYPE_IDENTIFIER &&
                    SF_FMT("Error: Member access operator (.) requires member name to be an identifier."));
                ret.v.member_access.child = (char *)strdup(arr->nodes[i + 1].v.Identifier.val);
                i++;
            }
        }
        break;
        default:
            break;
        }

        if (doBreak)
            break;
    }

    if (ret_now)
        return ret;

    return ret;
}

/**
 * MAIN:
 */
void SF_FrameIT_fromAST(mod_t *mod)
{
    for (size_t i = 0; i < mod->ast->size; i++)
    {
        psf_byte_t curr = mod->ast->nodes[i];

        switch (curr.nval_type)
        {
        case AST_NVAL_TYPE_OPERATOR:
        {
            const char *op = curr.v.Operator.val;

            if (!strcmp(op, "="))
            {
                psf_byte_array_t *val_arr = _PSF_newByteArray();
                int j = i + 1;

                while (mod->ast->nodes[j].nval_type != AST_NVAL_TYPE_NEWLINE && j < mod->ast->size)
                {
                    PSF_AST_ByteArray_AddNode(val_arr, mod->ast->nodes[j]);
                    j++;
                }

                expr_t *val_res = (expr_t *)OSF_Malloc(sizeof(expr_t));
                *val_res = SF_FrameExpr_fromByte(val_arr);

                stmt_t new_stmt;
                new_stmt.type = STATEMENT_TYPE_VAR_DECL;
                new_stmt.v.var_decl.name = OSF_Malloc(sizeof(expr_t));

                switch (BODY(mod)->body[BODY(mod)->body_size - 1].type)
                {
                case STATEMENT_TYPE_VAR_REF:
                {
                    *(new_stmt.v.var_decl.name) = (expr_t){
                        .type = EXPR_TYPE_VARIABLE,
                        .v = {
                            .variable = {
                                .name = BODY(mod)->body[BODY(mod)->body_size - 1].v.var_ref.name}}};
                }
                break;
                case STATEMENT_TYPE_EXPR:
                {
                    *(new_stmt.v.var_decl.name) = *(BODY(mod)->body[BODY(mod)->body_size - 1].v.expr.expr);
                }
                break;

                default:
                    break;
                }

                new_stmt.v.var_decl.expr = val_res;

                if (BODY(mod)->body_size)
                    BODY(mod)->body[BODY(mod)->body_size - 1] = new_stmt;
                else
                    PSG_AddStmt_toMod(mod, new_stmt);

                OSF_Free(val_arr);
                i = j;
            }
            else if (!strcmp(op, "("))
            {
                // IDENTIFIER ( VA_ARGS )
                stmt_t new_stmt;
                new_stmt.type = STATEMENT_TYPE_EXPR;
                new_stmt.v.expr.expr = (expr_t *)OSF_Malloc(sizeof(expr_t));
                new_stmt.v.expr.expr->type = EXPR_TYPE_FUNCTION_CALL;
                new_stmt.v.expr.expr->v.function_call.args = (expr_t *)OSF_Malloc(sizeof(expr_t));
                new_stmt.v.expr.expr->v.function_call.arg_size = 0;

                psf_byte_array_t *args_arr = _PSF_newByteArray();

                int j = i + 1, gb = 0;
                while (j < mod->ast->size)
                {
                    if (mod->ast->nodes[j].nval_type == AST_NVAL_TYPE_OPERATOR)
                    {
                        const char *op = strdup(mod->ast->nodes[j].v.Operator.val);

                        if (!strcmp(op, ")") && !gb)
                        {
                            if (args_arr->size)
                            {
                                expr_t arg_res = SF_FrameExpr_fromByte(args_arr);
                                if (new_stmt.v.expr.expr->v.function_call.arg_size)
                                    new_stmt.v.expr.expr->v.function_call.args = (expr_t *)OSF_Realloc(
                                        new_stmt.v.expr.expr->v.function_call.args,
                                        (new_stmt.v.expr.expr->v.function_call.arg_size + 1) *
                                            sizeof(expr_t));

                                new_stmt.v.expr.expr->v.function_call.args[new_stmt.v.expr.expr->v.function_call.arg_size++] = arg_res;
                            }
                            // printf("[%d]\n", new_stmt.v.expr.expr->v.function_call.arg_size);
                            OSF_Free(args_arr->nodes);
                            OSF_Free(args_arr);
                            break;
                        }

                        if (!strcmp(op, ",") && !gb)
                        {
                            expr_t arg_res = SF_FrameExpr_fromByte(args_arr);

                            if (new_stmt.v.expr.expr->v.function_call.arg_size)
                                new_stmt.v.expr.expr->v.function_call.args = (expr_t *)OSF_Realloc(
                                    new_stmt.v.expr.expr->v.function_call.args,
                                    (new_stmt.v.expr.expr->v.function_call.arg_size + 1) *
                                        sizeof(expr_t));

                            new_stmt.v.expr.expr->v.function_call.args[new_stmt.v.expr.expr->v.function_call.arg_size++] = arg_res;

                            OSF_Free(args_arr->nodes);
                            OSF_Free(args_arr);
                            args_arr = _PSF_newByteArray();
                            j++;
                            continue;
                        }

                        if (!strcmp(op, "(") ||
                            !strcmp(op, "{") ||
                            !strcmp(op, "["))
                            gb++;
                        else if (!strcmp(op, ")") ||
                                 !strcmp(op, "}") ||
                                 !strcmp(op, "]"))
                            gb--;
                    }

                    PSF_AST_ByteArray_AddNode(args_arr, mod->ast->nodes[j]);

                    j++;
                }

                new_stmt.v.expr.expr->v.function_call.name = OSF_Malloc(sizeof(expr_t));
                new_stmt.v.expr.expr->v.function_call.is_func_call = 1;

                if (BODY(mod)->body_size)
                {
                    stmt_t last_stmt = BODY(mod)->body[BODY(mod)->body_size - 1];

                    switch (last_stmt.type)
                    {
                    case STATEMENT_TYPE_VAR_REF:
                    {
                        const char *last_stmt_name = last_stmt.v.var_ref.name;

                        *(new_stmt.v.expr.expr->v.function_call.name) = (expr_t){
                            .type = EXPR_TYPE_VARIABLE,
                            .v.variable.name = (char *)strdup(last_stmt_name)};
                    }
                    break;
                    case STATEMENT_TYPE_EXPR:
                    {
                        if (last_stmt.v.expr.expr->type == EXPR_TYPE_MEMBER_ACCESS)
                        {
                            *(new_stmt.v.expr.expr->v.function_call.name) = *(last_stmt.v.expr.expr);
                        }
                    }
                    default:
                        break;
                    }
                }
                else
                {
                    new_stmt.v.expr.expr->v.function_call.is_func_call = 0;
                    *(new_stmt.v.expr.expr->v.function_call.name) = (expr_t){
                        .type = EXPR_TYPE_VARIABLE,
                        .v.variable.name = NULL};
                }

                if (BODY(mod)->body_size)
                    BODY(mod)->body[BODY(mod)->body_size - 1] = new_stmt;
                else
                    PSG_AddStmt_toMod(mod, new_stmt);

                i = j;
                continue;
            }
            else if (!strcmp(op, "."))
            {
                stmt_t new_stmt;
                new_stmt.type = STATEMENT_TYPE_EXPR;
                new_stmt.v.expr.expr = OSF_Malloc(sizeof(expr_t));
                new_stmt.v.expr.expr->type = EXPR_TYPE_MEMBER_ACCESS;

                switch (BODY(mod)->body[BODY(mod)->body_size - 1].type)
                {
                case STATEMENT_TYPE_VAR_REF:
                {
                    new_stmt.v.expr.expr->v.member_access.parent = OSF_Malloc(sizeof(expr_t));
                    *(new_stmt.v.expr.expr->v.member_access.parent) = (expr_t){
                        .type = EXPR_TYPE_VARIABLE,
                        .v = {
                            .variable = {
                                .name = BODY(mod)->body[BODY(mod)->body_size - 1].v.var_ref.name}}};
                }
                break;

                default:
                    break;
                }

                switch (mod->ast->nodes[i + 1].nval_type)
                {
                case AST_NVAL_TYPE_IDENTIFIER:
                {
                    new_stmt.v.expr.expr->v.member_access.child = (char *)strdup(mod->ast->nodes[i + 1].v.Identifier.val);
                }
                break;

                default:
                    break;
                }

                BODY(mod)->body[BODY(mod)->body_size - 1] = new_stmt;
                i++;
            }
        }
        break;
        case AST_NVAL_TYPE_IDENTIFIER:
        {
            if (!curr.v.Identifier.is_token)
            {
                stmt_t new_stmt;
                new_stmt.type = STATEMENT_TYPE_VAR_REF;
                new_stmt.v.var_ref.name = (char *)strdup(curr.v.Identifier.val);

                PSG_AddStmt_toMod(mod, new_stmt);
            }
            else
            {
                const char *tok = curr.v.Identifier.val;
                stmt_t new_stmt;

                if (!strcmp(tok, "if"))
                {
                    new_stmt.type = STATEMENT_TYPE_IF;
                    psf_byte_array_t *cond_arr = _PSF_newByteArray();
                    int gb = 0, nl_st = -1;

                    for (size_t j = i + 1; j < mod->ast->size; j++)
                    {
                        psf_byte_t c = mod->ast->nodes[j];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (!strcmp(c.v.Operator.val, ")") ||
                                     !strcmp(c.v.Operator.val, "}") ||
                                     !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                        {
                            nl_st = j;
                            break;
                        }

                        PSF_AST_ByteArray_AddNode(cond_arr, c);
                    }

                    assert(nl_st != -1 && SF_FMT("Error: Syntax Error"));

                    new_stmt.v.if_stmt.condition = (expr_t *)OSF_Malloc(sizeof(expr_t));
                    *(new_stmt.v.if_stmt.condition) = SF_FrameExpr_fromByte(cond_arr);

                    psf_byte_array_t *body_arr = _PSF_GetBody(mod->ast, nl_st, _PSF_GetTabspace(mod->ast, i));

                    mod_t *b_m = SF_CreateModule(MODULE_TYPE_FILE, body_arr);
                    SF_FrameIT_fromAST(b_m);

                    new_stmt.v.if_stmt.body = BODY(b_m)->body;
                    new_stmt.v.if_stmt.body_size = BODY(b_m)->body_size;
                    new_stmt.v.if_stmt.elif_stmts_count = 0;
                    new_stmt.v.if_stmt.elif_stmts = OSF_Malloc(sizeof(struct _conditional_struct));
                    new_stmt.v.if_stmt.else_stmt = OSF_Malloc(sizeof(struct _conditional_struct));

                    i = nl_st + b_m->ast->size;
                    OSF_Free(b_m->var_holds);
                    OSF_Free(b_m);
                    OSF_Free(cond_arr->nodes);
                    OSF_Free(cond_arr);
                    OSF_Free(body_arr->nodes);
                    OSF_Free(body_arr);

                    PSG_AddStmt_toMod(mod, new_stmt);
                }
                else if (!strcmp(tok, "else"))
                {
                    assert(BODY(mod)->body[BODY(mod)->body_size - 1].type == STATEMENT_TYPE_IF && SF_FMT("Error: Stray clause"));
                    if (mod->ast->nodes[i + 1].nval_type == AST_NVAL_TYPE_IDENTIFIER &&
                        !strcmp(mod->ast->nodes[i + 1].v.Identifier.val, "if"))
                    {
                        psf_byte_array_t *cond_arr = _PSF_newByteArray();
                        int gb = 0, nl_st = -1;

                        for (size_t j = i + 2; j < mod->ast->size; j++)
                        {
                            psf_byte_t c = mod->ast->nodes[j];

                            if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                            {
                                if (!strcmp(c.v.Operator.val, "(") ||
                                    !strcmp(c.v.Operator.val, "{") ||
                                    !strcmp(c.v.Operator.val, "["))
                                    gb++;
                                else if (!strcmp(c.v.Operator.val, ")") ||
                                         !strcmp(c.v.Operator.val, "}") ||
                                         !strcmp(c.v.Operator.val, "]"))
                                    gb--;
                            }

                            if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                            {
                                nl_st = j;
                                break;
                            }

                            PSF_AST_ByteArray_AddNode(cond_arr, c);
                        }

                        assert(nl_st != -1 && SF_FMT("Error: Syntax Error"));

                        if (BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count)
                            BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts = (struct _conditional_struct *)OSF_Realloc(
                                BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts,
                                (BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count + 1) *
                                    sizeof(struct _conditional_struct));

                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].condition = OSF_Malloc(sizeof(expr_t));
                        *(BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].condition) = SF_FrameExpr_fromByte(cond_arr);

                        psf_byte_array_t *body_arr = _PSF_GetBody(mod->ast, nl_st, _PSF_GetTabspace(mod->ast, i));

                        mod_t *b_m = SF_CreateModule(MODULE_TYPE_FILE, body_arr);
                        SF_FrameIT_fromAST(b_m);

                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].body = BODY(b_m)->body;
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].body_size = BODY(b_m)->body_size;
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].elif_stmts = OSF_Malloc(sizeof(struct _conditional_struct));
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].elif_stmts_count = 0;
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts[BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count].else_stmt = OSF_Malloc(sizeof(struct _conditional_struct));

                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.elif_stmts_count++;

                        i = nl_st + b_m->ast->size;
                        OSF_Free(b_m);
                        OSF_Free(cond_arr->nodes);
                        OSF_Free(cond_arr);
                        OSF_Free(body_arr->nodes);
                        OSF_Free(body_arr);
                    }
                    else
                    {
                        psf_byte_array_t *body_arr = _PSF_GetBody(mod->ast, i + 1, _PSF_GetTabspace(mod->ast, i));

                        mod_t *b_m = SF_CreateModule(MODULE_TYPE_FILE, body_arr);
                        SF_FrameIT_fromAST(b_m);

                        if (BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt == NULL)
                            BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt = OSF_Malloc(sizeof(struct _conditional_struct));

                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt->body = BODY(b_m)->body;
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt->body_size = BODY(b_m)->body_size;
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt->elif_stmts = OSF_Malloc(sizeof(struct _conditional_struct));
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt->elif_stmts_count = 0;
                        BODY(mod)->body[BODY(mod)->body_size - 1].v.if_stmt.else_stmt->else_stmt = OSF_Malloc(sizeof(struct _conditional_struct));

                        i += b_m->ast->size + 1;
                        OSF_Free(b_m);
                        OSF_Free(body_arr->nodes);
                        OSF_Free(body_arr);
                    }
                }
                else if (!strcmp(tok, "while"))
                {
                    new_stmt.type = STATEMENT_TYPE_WHILE;
                    psf_byte_array_t *cond_arr = _PSF_newByteArray();
                    int gb = 0, nl_st = -1;

                    for (size_t j = i + 1; j < mod->ast->size; j++)
                    {
                        psf_byte_t c = mod->ast->nodes[j];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (!strcmp(c.v.Operator.val, ")") ||
                                     !strcmp(c.v.Operator.val, "}") ||
                                     !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                        {
                            nl_st = j;
                            break;
                        }

                        PSF_AST_ByteArray_AddNode(cond_arr, c);
                    }

                    assert(nl_st != -1 && SF_FMT("Error: Syntax Error"));

                    new_stmt.v.while_stmt.condition = (expr_t *)OSF_Malloc(sizeof(expr_t));
                    *(new_stmt.v.while_stmt.condition) = SF_FrameExpr_fromByte(cond_arr);

                    psf_byte_array_t *body_arr = _PSF_GetBody(mod->ast, nl_st, _PSF_GetTabspace(mod->ast, i));

                    mod_t *b_m = SF_CreateModule(MODULE_TYPE_FILE, body_arr);
                    SF_FrameIT_fromAST(b_m);

                    new_stmt.v.while_stmt.body = BODY(b_m)->body;
                    new_stmt.v.while_stmt.body_size = BODY(b_m)->body_size;

                    i = nl_st + b_m->ast->size;
                    OSF_Free(b_m);
                    OSF_Free(cond_arr->nodes);
                    OSF_Free(cond_arr);
                    OSF_Free(body_arr->nodes);
                    OSF_Free(body_arr);

                    PSG_AddStmt_toMod(mod, new_stmt);
                }
                else if (!strcmp(tok, "for"))
                {
                    int body_sz = 0;
                    new_stmt = _PSF_ConstructForLoopStmt(mod, i, &body_sz);
                    PSG_AddStmt_toMod(mod, new_stmt);

                    int gb = 0;
                    while (i < mod->ast->size)
                    {
                        psf_byte_t c = mod->ast->nodes[i];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (!strcmp(c.v.Operator.val, ")") ||
                                     !strcmp(c.v.Operator.val, "}") ||
                                     !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                            break;

                        i++;
                    }

                    i += body_sz;
                }
                else if (!strcmp(tok, "repeat"))
                {
                    int bd_sz = 0;
                    new_stmt = _PSF_ConstructRepeatLoopStmt(mod, i + 1, &bd_sz);
                    PSG_AddStmt_toMod(mod, new_stmt);

                    int gb = 0;
                    while (i < mod->ast->size)
                    {
                        psf_byte_t c = mod->ast->nodes[i];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (!strcmp(c.v.Operator.val, ")") ||
                                     !strcmp(c.v.Operator.val, "}") ||
                                     !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                            break;

                        i++;
                    }

                    i += bd_sz;
                }
                else if (!strcmp(tok, "fun"))
                {
                    int bd_sz = 0;
                    new_stmt = _PSF_ConstructFunctionStmt(mod->ast, i, &bd_sz);
                    PSG_AddStmt_toMod(mod, new_stmt);

                    int gb = 0;
                    while (i < mod->ast->size)
                    {
                        psf_byte_t c = mod->ast->nodes[i];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (!strcmp(c.v.Operator.val, ")") ||
                                     !strcmp(c.v.Operator.val, "}") ||
                                     !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                            break;

                        i++;
                    }

                    i += bd_sz;
                }
                else if (!strcmp(tok, "class"))
                {
                    new_stmt = _PSF_ConstructClassStmt(mod->ast, i + 1, &i);
                    PSG_AddStmt_toMod(mod, new_stmt);
                }
                else if (!strcmp(tok, "return"))
                {
                    new_stmt.type = STATEMENT_TYPE_RETURN;
                    psf_byte_array_t *ret_arr = _PSF_newByteArray();
                    int gb = 0;

                    while (i < mod->ast->size)
                    {
                        psf_byte_t c = mod->ast->nodes[i];

                        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
                        {
                            if (!strcmp(c.v.Operator.val, "(") ||
                                !strcmp(c.v.Operator.val, "{") ||
                                !strcmp(c.v.Operator.val, "["))
                                gb++;
                            else if (!strcmp(c.v.Operator.val, ")") ||
                                     !strcmp(c.v.Operator.val, "}") ||
                                     !strcmp(c.v.Operator.val, "]"))
                                gb--;
                        }

                        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                            break;

                        PSF_AST_ByteArray_AddNode(ret_arr, c);
                        i++;
                    }

                    new_stmt.v.return_stmt.expr = OSF_Malloc(sizeof(expr_t));

                    if (!ret_arr->size)
                        *(new_stmt.v.return_stmt.expr) = (expr_t){
                            .type = EXPR_TYPE_CONSTANT,
                            .v = {
                                .constant = {
                                    .constant_type = CONSTANT_TYPE_DTYPE,
                                    .DType = {
                                        .type = DATA_TYPE_NONE}}}};
                    else
                        *(new_stmt.v.return_stmt.expr) = SF_FrameExpr_fromByte(ret_arr);

                    OSF_Free(ret_arr->nodes);
                    OSF_Free(ret_arr);

                    PSG_AddStmt_toMod(mod, new_stmt);
                }
            }
        }
        break;
        default:
            break;
        }
    }
}

void PSG_PrintStatementType(enum StatementTypeEnum _en)
{
    const char *A[] = {
        "STATEMENT_NOT_IMPLEMENTED",
        "STATEMENT_TYPE_EXPR",
        "STATEMENT_TYPE_IF",
        "STATEMENT_TYPE_ELIF",
        "STATEMENT_TYPE_ELSE",
        "STATEMENT_TYPE_WHILE",
        "STATEMENT_TYPE_FOR",
        "STATEMENT_TYPE_RETURN",
        "STATEMENT_TYPE_BREAK",
        "STATEMENT_TYPE_CONTINUE",
        "STATEMENT_TYPE_VAR_DECL",
        "STATEMENT_TYPE_FUNCTION_DECL",
        "STATEMENT_TYPE_CLASS_DECL",
        "STATEMENT_TYPE_IMPORT",
        "STATEMENT_TYPE_COMMENT",
        "STATEMENT_TYPE_VAR_REF",
        "STATEMENT_TYPE_REPEAT"};

    printf("%s(%d)\n", A[_en + 1 < (sizeof(A) / sizeof(A[0])) ? _en + 1 : 0], _en);
}

void *SF_CCast_Entity(expr_t en)
{
    switch (en.type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        switch (en.v.constant.constant_type)
        {
        case CONSTANT_TYPE_BOOL:
            return (void *)(int *)(int[]){(en.v.constant.Bool.value)};
            break;
        case CONSTANT_TYPE_DTYPE:
            switch (en.v.constant.DType.type)
            {
            case DATA_TYPE_NONE:
                return NULL;
                break;

            default:
                return NULL;
                break;
            }
            break;
        case CONSTANT_TYPE_FLOAT:
            if (en.v.constant.Float.is_inf)
                return (void *)(int *)(int[]){INT_MAX};
            return (void *)(float *)(float[]){(en.v.constant.Float.value)};
            break;
        case CONSTANT_TYPE_INT:
            return (void *)(int *)(int[]){(en.v.constant.Int.value)};
            break;
        case CONSTANT_TYPE_STRING:
        {
            char *res = (char *)strdup(en.v.constant.String.value);
            res[strlen(res) - 1] = '\0';
            res++;
            return (void *)(const char *)res;
        }
        break;

        default:
            return NULL;
            break;
        }
    }
    break;
    // case EXPR_TYPE_VARIABLE:
    //     return &(en.v.variable);
    //     break;
    // case EXPR_TYPE_FUNCTION:
    //     return &(en.v.function);
    //     break;
    // case EXPR_TYPE_CLASS:
    //     return &(en.v.class);
    //     break;
    // case EXPR_TYPE_OPERATOR:
    //     return &(en.v.operator);
    //     break;
    default:
        break;
    }

    return NULL;
}

void PSG_AddStmt_toMod(mod_t *mod, stmt_t stmt)
{
    if (BODY(mod)->body_size)
        BODY(mod)->body = (struct _stmt *)OSF_Realloc(BODY(mod)->body, (BODY(mod)->body_size + 1) * sizeof(struct _stmt));

    BODY(mod)->body[BODY(mod)->body_size++] = stmt;
}

void PSG_PrintExprType(enum ExprTypeEnum _en)
{
    const char *A[] = {
        "EXPR_NOT_IMPLEMENTED",
        "EXPR_TYPE_CONSTANT",
        "EXPR_TYPE_VARIABLE",
        "EXPR_TYPE_FUNCTION_CALL",
        "EXPR_TYPE_FUNCTION",
        "EXPR_TYPE_INLINE_ASSIGNMENT",
        "EXPR_TYPE_LOGICAL_ARITHMETIC_OP",
        "EXPR_TYPE_TO_STEP_CLAUSE",
        "EXPR_TYPE_INDEX_OP",
        "EXPR_TYPE_INLINE_FOR",
        "EXPR_TYPE_UNARY_ARITHMETIC_OP",
        "EXPR_TYPE_THEN_WHILE",
        "EXPR_TYPE_INLINE_REPEAT",
        "EXPR_TYPE_CLASS",
        "EXPR_TYPE_MEMBER_ACCESS"};

    printf("%s(%d)\n", A[_en + 1 < (sizeof(A) / sizeof(A[0])) ? _en + 1 : 0], _en);
}

fun_t **PSG_GetFunctions(void)
{
    return &PSG_Functions;
}

int *PSG_GetFunctionsSize(void)
{
    return &PSG_Functions_Size;
}

void _PSG_EnvInit(void)
{
    PSG_Functions = (fun_t *)OSF_Malloc(sizeof(fun_t));
    PSG_Functions_Size = 0;
    PSG_Classes = (class_t *)OSF_Malloc(sizeof(class_t));
    PSG_Classes_Size = 0;
}

int PSG_AddFunction(fun_t _fun)
{
    if (*PSG_GetFunctionsSize())
        *PSG_GetFunctions() = (fun_t *)OSF_Realloc(*PSG_GetFunctions(), (*PSG_GetFunctionsSize() + 1) * sizeof(fun_t));

    (*PSG_GetFunctions())[*PSG_GetFunctionsSize()] = _fun;

    return (*PSG_GetFunctionsSize())++;
}

int _PSF_GetTabspace(psf_byte_array_t *arr, int st)
{
    for (int i = st; i >= 0; i--)
    {
        if (arr->nodes[i].nval_type == AST_NVAL_TYPE_NEWLINE)
            return 0;
        if (arr->nodes[i].nval_type == AST_NVAL_TYPE_TABSPACE)
            return arr->nodes[i].v.Tabspace.len;
    }

    return 0;
}

psf_byte_array_t *_PSF_GetBody(psf_byte_array_t *arr, int st, int tab)
{
    psf_byte_array_t *res = _PSF_newByteArray();

    for (size_t i = st; i < arr->size; i++)
    {
        psf_byte_t c = arr->nodes[i];

        // if (c.nval_type == AST_NVAL_TYPE_TABSPACE)
        // {
        //     if (c.v.Tabspace.len < tab)
        //         break;
        // }
        if (c.nval_type == AST_NVAL_TYPE_NEWLINE)
        {
            int ii = i + 1;
            while (ii < arr->size && arr->nodes[ii].nval_type == AST_NVAL_TYPE_NEWLINE)
                ii++;

            if (ii < arr->size)
            {
                if (arr->nodes[ii].nval_type == AST_NVAL_TYPE_TABSPACE)
                {
                    if (arr->nodes[ii].v.Tabspace.len <= tab)
                        break;
                }
                else
                    break;
            }
            else
                break;
        }

        PSF_AST_ByteArray_AddNode(res, c);
    }

    return res;
}

int _PSF_EntityIsTrue(expr_t ent)
{
    switch (ent.type)
    {
    case EXPR_TYPE_CONSTANT:
    {
        switch (ent.v.constant.constant_type)
        {
        case CONSTANT_TYPE_BOOL:
            return ent.v.constant.Bool.value;
            break;
        case CONSTANT_TYPE_DTYPE:
        {
            if (ent.v.constant.DType.type == DATA_TYPE_NONE)
                return 0;
            else if (ent.v.constant.DType.type == DATA_TYPE_VOID) // sus
                return 0;
        }
        break;
        case CONSTANT_TYPE_FLOAT:
        {
            if (ent.v.constant.Float.is_inf)
                return 1;
            if (ent.v.constant.Float.value == 0.0)
                return 0;
            else
                return 1;
        }
        break;
        case CONSTANT_TYPE_INT:
            return !!ent.v.constant.Int.value;
            break;
        case CONSTANT_TYPE_STRING:
            return !!strlen(ent.v.constant.String.value);
            break;
        default:
            return 0;
            break;
        }
    }
    break;

    default:
        break;
    }

    return 0;
}

expr_t *_PSF_RemoveVoidsFromExprArray(expr_t *arr, int size, int *sz_ptr)
{
    expr_t *new_arr = OSF_Malloc(sizeof(expr_t));
    int new_arr_count = 0;

    for (size_t i = 0; i < size; i++)
    {
        if (arr[i].type == EXPR_TYPE_CONSTANT)
            if (arr[i].v.constant.constant_type == CONSTANT_TYPE_DTYPE)
                if (arr[i].v.constant.DType.type == DATA_TYPE_VOID)
                    continue;

        if (new_arr_count)
            new_arr = OSF_Realloc(new_arr, (new_arr_count + 1) * sizeof(expr_t));

        new_arr[new_arr_count++] = arr[i];
    }

    if (sz_ptr != NULL)
        *sz_ptr = new_arr_count;

    return new_arr;
}

stmt_t _PSF_ConstructForLoopStmt(mod_t *mod, int idx, int *body_sz_ptr)
{
    assert(
        (mod->ast->nodes[idx].nval_type == AST_NVAL_TYPE_IDENTIFIER &&
         mod->ast->nodes[idx].v.Identifier.is_token &&
         !strcmp(mod->ast->nodes[idx].v.Identifier.val, "for")) &&
        SF_FMT("Error: Syntax Error."));

    var_t *vars = OSF_Malloc(sizeof(var_t));
    int vars_size = 0, gb = 0;

    psf_byte_array_t *var_top = _PSF_newByteArray();

    // Fetch variables
    int i = idx + 1;
    while (i < mod->ast->size)
    {
        psf_byte_t c = mod->ast->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (
                !strcmp(c.v.Operator.val, ")") ||
                !strcmp(c.v.Operator.val, "}") ||
                !strcmp(c.v.Operator.val, "]"))
                gb--;

            if (!strcmp(c.v.Operator.val, ",") &&
                !gb)
            {
                if (vars_size)
                    vars = OSF_Realloc(vars, (vars_size + 1) * sizeof(var_t));

                vars[vars_size++] = _PSF_GenerateVarFromByte(var_top);

                OSF_Free(var_top->nodes);
                OSF_Free(var_top);
                var_top = _PSF_newByteArray();
                i++;
                continue;
            }
        }

        if (c.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
            c.v.Identifier.is_token &&
            !strcmp(c.v.Identifier.val, "in") &&
            !gb)
        {
            if (vars_size)
                vars = OSF_Realloc(vars, (vars_size + 1) * sizeof(var_t));

            vars[vars_size++] = _PSF_GenerateVarFromByte(var_top);

            break;
        }

        PSF_AST_ByteArray_AddNode(var_top, c);
        i++;
    }
    i++; // Eat 'in'
    gb = 0;

    // Fetch condition
    psf_byte_array_t *cond_top = _PSF_newByteArray();

    while (i < mod->ast->size)
    {
        psf_byte_t c = mod->ast->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (
                !strcmp(c.v.Operator.val, ")") ||
                !strcmp(c.v.Operator.val, "}") ||
                !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        if (c.nval_type == AST_NVAL_TYPE_NEWLINE &&
            !gb)
            break;

        PSF_AST_ByteArray_AddNode(cond_top, c);
        i++;
    }

    // Fetch body
    psf_byte_array_t *body_top = _PSF_GetBody(mod->ast, i, _PSF_GetTabspace(mod->ast, idx));
    mod_t *body_mod = SF_CreateModule(MODULE_TYPE_FILE, body_top);
    body_mod->ast = body_top;

    if (body_sz_ptr != NULL)
        *body_sz_ptr = body_mod->ast->size;

    SF_FrameIT_fromAST(body_mod);

    // Construct for loop
    stmt_t stmt = {
        .type = STATEMENT_TYPE_FOR,
        .v = {
            .for_stmt = {
                .body = BODY(body_mod)->body,
                .body_size = BODY(body_mod)->body_size,
                .condition = OSF_Malloc(sizeof(expr_t)),
                .var_size = vars_size,
                .vars = vars}}};

    *(stmt.v.for_stmt.condition) = SF_FrameExpr_fromByte(cond_top);

    OSF_Free(var_top->nodes);
    OSF_Free(var_top);
    OSF_Free(cond_top->nodes);
    OSF_Free(cond_top);
    OSF_Free(body_top->nodes);
    OSF_Free(body_top);
    OSF_Free(body_mod->var_holds);
    OSF_Free(body_mod);

    return stmt;
}

expr_t _PSF_ConstructInlineForExpr(psf_byte_array_t *arr, int idx)
{
    assert(
        (arr->nodes[idx].nval_type == AST_NVAL_TYPE_IDENTIFIER &&
         arr->nodes[idx].v.Identifier.is_token &&
         !strcmp(arr->nodes[idx].v.Identifier.val, "for")) &&
        SF_FMT("Error: Syntax Error."));

    expr_t res;
    res.type = EXPR_TYPE_INLINE_FOR;

    psf_byte_array_t *body_arr = _PSF_newByteArray(),
                     *cond_arr = _PSF_newByteArray(),
                     *vars_arr = _PSF_newByteArray();

    for (size_t i = 0; i < idx; i++)
        PSF_AST_ByteArray_AddNode(body_arr, arr->nodes[i]);

    var_t *vars = OSF_Malloc(sizeof(var_t));
    int vars_size = 0;

    int i = idx + 1;
    int gb = 0;
    while (i < arr->size)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (
                !strcmp(c.v.Operator.val, ")") ||
                !strcmp(c.v.Operator.val, "}") ||
                !strcmp(c.v.Operator.val, "]"))
                gb--;

            if (!strcmp(c.v.Operator.val, ",") &&
                !gb)
            {
                if (vars_size)
                    vars = OSF_Realloc(vars, (vars_size + 1) * sizeof(var_t));

                vars[vars_size++] = _PSF_GenerateVarFromByte(vars_arr);

                OSF_Free(vars_arr->nodes);
                OSF_Free(vars_arr);
                vars_arr = _PSF_newByteArray();
                i++;
                continue;
            }
        }

        if (c.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
            c.v.Identifier.is_token &&
            !strcmp(c.v.Identifier.val, "in") &&
            !gb)
        {
            if (vars_size)
                vars = OSF_Realloc(vars, (vars_size + 1) * sizeof(var_t));

            vars[vars_size++] = _PSF_GenerateVarFromByte(vars_arr);

            break;
        }

        PSF_AST_ByteArray_AddNode(vars_arr, c);
        i++;
    }
    i++; // Eat 'in'
    gb = 0;

    while (i < arr->size)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (
                !strcmp(c.v.Operator.val, ")") ||
                !strcmp(c.v.Operator.val, "}") ||
                !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        if (c.nval_type == AST_NVAL_TYPE_NEWLINE &&
            !gb)
            break;

        PSF_AST_ByteArray_AddNode(cond_arr, c);
        i++;
    }

    res.v.inline_for.body = OSF_Malloc(sizeof(expr_t));
    res.v.inline_for.condition = OSF_Malloc(sizeof(expr_t));
    res.v.inline_for.var_size = vars_size;
    res.v.inline_for.vars = (void *)vars;

    *(res.v.inline_for.body) = SF_FrameExpr_fromByte(body_arr);
    *(res.v.inline_for.condition) = SF_FrameExpr_fromByte(cond_arr);

    OSF_Free(body_arr->nodes);
    OSF_Free(body_arr);
    OSF_Free(cond_arr->nodes);
    OSF_Free(cond_arr);

    return res;
}

var_t _PSF_GenerateVarFromByte(psf_byte_array_t *arr)
{
    var_t res;
    int gb = 0;
    int gb_at_varname = 0;
    int got_val = 0;

    for (size_t i = 0; i < arr->size; i++)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;

            if (!strcmp(c.v.Operator.val, "=") &&
                gb == gb_at_varname)
            {
                // Value for variable
                psf_byte_array_t *val_arr = _PSF_newByteArray();
                for (size_t j = i + 1; j < arr->size; j++)
                    PSF_AST_ByteArray_AddNode(val_arr, arr->nodes[j]);

                res.val = SF_FrameExpr_fromByte(val_arr);
                got_val = 1;

                OSF_Free(val_arr);
                break;
            }
        }

        if (c.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
            !c.v.Identifier.is_token)
        {
            res.name = strdup(c.v.Identifier.val);
            gb_at_varname = gb;
        }
    }

    if (!got_val)
    {
        res.val = (expr_t){
            .type = EXPR_TYPE_CONSTANT,
            .v = {
                .constant = {
                    .constant_type = CONSTANT_TYPE_DTYPE,
                    .DType = DATA_TYPE_VOID}}};
    }

    return res;
}

expr_t _PSF_ConstructWhileThenExpr(psf_byte_array_t *arr, int idx)
{
    psf_byte_array_t *body = _PSF_newByteArray(),
                     *assigns_arr = _PSF_newByteArray(),
                     *condition = _PSF_newByteArray(),
                     *withs_arr = _PSF_newByteArray();

    var_t *assigns = OSF_Malloc(sizeof(var_t));
    int as_count = 0;

    var_t *withs = OSF_Malloc(sizeof(var_t));
    int w_count = 0;

    expr_t res;
    res.type = EXPR_TYPE_THEN_WHILE;

    int gb = 0;
    int i = 0;

    while (i < arr->size)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        if (c.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
            c.v.Identifier.is_token &&
            !strcmp(c.v.Identifier.val, "with") &&
            !gb)
            break;

        PSF_AST_ByteArray_AddNode(body, c);
        i++;
    }

    i++; // Eat 'with'
    gb = 0;

    while (i < arr->size)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;

            if (!strcmp(c.v.Operator.val, ",") &&
                !gb)
            {
                if (w_count)
                    withs = OSF_Realloc(withs, (w_count + 1) * sizeof(var_t));

                withs[w_count++] = _PSF_GenerateVarFromByte(withs_arr);

                OSF_Free(withs_arr->nodes);
                OSF_Free(withs_arr);
                withs_arr = _PSF_newByteArray();
                i++; // Eat ','
                continue;
            }
        }

        if (c.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
            c.v.Identifier.is_token &&
            !strcmp(c.v.Identifier.val, "then") &&
            !gb)
        {
            if (w_count)
                withs = OSF_Realloc(withs, (w_count + 1) * sizeof(var_t));

            withs[w_count++] = _PSF_GenerateVarFromByte(withs_arr);

            OSF_Free(withs_arr->nodes);
            OSF_Free(withs_arr);
            withs_arr = _PSF_newByteArray();
            break;
        }

        PSF_AST_ByteArray_AddNode(withs_arr, c);
        i++;
    }

    i++; // Eat 'then'
    gb = 0;

    while (i < arr->size)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;

            if (!strcmp(c.v.Operator.val, ",") &&
                !gb)
            {
                if (as_count)
                    assigns = OSF_Realloc(assigns, (as_count + 1) * sizeof(var_t));

                assigns[as_count++] = _PSF_GenerateVarFromByte(assigns_arr);

                OSF_Free(assigns_arr->nodes);
                OSF_Free(assigns_arr);
                assigns_arr = _PSF_newByteArray();
                i++; // Eat ','
                continue;
            }
        }

        if (c.nval_type == AST_NVAL_TYPE_IDENTIFIER &&
            c.v.Identifier.is_token &&
            !strcmp(c.v.Identifier.val, "while") &&
            !gb)
        {
            if (as_count)
                assigns = OSF_Realloc(assigns, (as_count + 1) * sizeof(var_t));

            assigns[as_count++] = _PSF_GenerateVarFromByte(assigns_arr);

            OSF_Free(assigns_arr->nodes);
            OSF_Free(assigns_arr);
            assigns_arr = _PSF_newByteArray();
            break;
        }

        PSF_AST_ByteArray_AddNode(assigns_arr, c);
        i++;
    }

    i++; // Eat 'while'
    gb = 0;

    while (i < arr->size)
    {
        psf_byte_t c = arr->nodes[i];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        PSF_AST_ByteArray_AddNode(condition, c);
        i++;
    }
    res.v.then_while.body = OSF_Malloc(sizeof(expr_t));
    res.v.then_while.condition = OSF_Malloc(sizeof(expr_t));

    res.v.then_while.assigns = (void *)assigns;
    res.v.then_while.assigns_size = as_count;
    *(res.v.then_while.body) = SF_FrameExpr_fromByte(body);
    *(res.v.then_while.condition) = SF_FrameExpr_fromByte(condition);
    res.v.then_while.withs = (void *)withs;
    res.v.then_while.withs_size = w_count;

    OSF_Free(body->nodes);
    OSF_Free(body);
    OSF_Free(assigns_arr->nodes);
    OSF_Free(assigns_arr);
    OSF_Free(condition->nodes);
    OSF_Free(condition);
    OSF_Free(withs_arr->nodes);
    OSF_Free(withs_arr);

    return res;
}

stmt_t _PSF_ConstructRepeatLoopStmt(mod_t *mod, int idx, int *bd_sz_ptr)
{
    stmt_t result;
    result.type = STATEMENT_TYPE_REPEAT;
    psf_byte_array_t *cond_arr = _PSF_newByteArray();
    int gb = 0;

    while (idx < mod->ast->size)
    {
        psf_byte_t c = mod->ast->nodes[idx];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
            break;

        PSF_AST_ByteArray_AddNode(cond_arr, c);
        idx++;
    }

    psf_byte_array_t *body_arr = _PSF_GetBody(mod->ast, idx, _PSF_GetTabspace(mod->ast, idx - 1));
    mod_t *body_mod = SF_CreateModule(MODULE_TYPE_FILE, body_arr);
    SF_FrameIT_fromAST(body_mod);

    result.v.repeat_stmt.body = BODY(body_mod)->body;
    result.v.repeat_stmt.body_size = BODY(body_mod)->body_size;
    result.v.repeat_stmt.condition = OSF_Malloc(sizeof(expr_t));
    *(result.v.repeat_stmt.condition) = SF_FrameExpr_fromByte(cond_arr);

    if (bd_sz_ptr != NULL)
        *bd_sz_ptr = body_arr->size;

    OSF_Free(body_mod->var_holds);
    OSF_Free(body_mod);

    OSF_Free(body_arr->nodes);
    OSF_Free(body_arr);

    OSF_Free(cond_arr->nodes);
    OSF_Free(cond_arr);

    return result;
}

expr_t _PSF_ConstructRepeatExpr(psf_byte_array_t *arr, int idx)
{
    expr_t res;
    res.type = EXPR_TYPE_INLINE_REPEAT;

    psf_byte_array_t *cond = _PSF_newByteArray(),
                     *body = _PSF_newByteArray();

    for (size_t i = 0; i < idx; i++)
        PSF_AST_ByteArray_AddNode(body, arr->nodes[i]);

    for (size_t i = idx + 1; i < arr->size; i++)
        PSF_AST_ByteArray_AddNode(cond, arr->nodes[i]);

    res.v.inline_repeat.body = OSF_Malloc(sizeof(expr_t));
    *(res.v.inline_repeat.body) = SF_FrameExpr_fromByte(body);
    res.v.inline_repeat.cond = OSF_Malloc(sizeof(expr_t));
    *(res.v.inline_repeat.cond) = SF_FrameExpr_fromByte(cond);

    OSF_Free(cond->nodes);
    OSF_Free(cond);
    OSF_Free(body->nodes);
    OSF_Free(body);

    return res;
}

stmt_t _PSF_ConstructFunctionStmt(psf_byte_array_t *arr, int idx, int *bd_sz_ptr)
{
    stmt_t result;
    result.type = STATEMENT_TYPE_FUNCTION_DECL;
    result.v.function_decl.args = OSF_Malloc(sizeof(var_t));
    result.v.function_decl.arg_size = 0;

    psf_byte_array_t *name = _PSF_newByteArray(),
                     *args = _PSF_newByteArray(),
                     *body;
    int gb = 0;
    int takes_arguments = 0;
    idx++; // Eat 'fun'

    while (idx < arr->size)
    {
        psf_byte_t c = arr->nodes[idx];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") &&
                !gb)
            {
                takes_arguments = 1;
                break;
            }
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
            break;

        PSF_AST_ByteArray_AddNode(name, c);
        idx++;
    }

    if (takes_arguments)
    {
        idx++; // Eat '('
        gb = 0;

        while (idx < arr->size)
        {
            psf_byte_t c = arr->nodes[idx];

            if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
            {
                if (!strcmp(c.v.Operator.val, ")") && !gb)
                {
                    if (args->size)
                    {
                        var_t _Gv = _PSF_GenerateVarFromByte(args);

                        if (result.v.function_decl.arg_size)
                            result.v.function_decl.args = OSF_Realloc(result.v.function_decl.args, (result.v.function_decl.arg_size + 1) * sizeof(var_t));

                        result.v.function_decl.args[result.v.function_decl.arg_size++] = _Gv;

                        OSF_Free(args->nodes);
                        OSF_Free(args);
                        args = _PSF_newByteArray();
                    }
                    break;
                }
                if (!strcmp(c.v.Operator.val, "(") ||
                    !strcmp(c.v.Operator.val, "{") ||
                    !strcmp(c.v.Operator.val, "["))
                    gb++;
                else if (!strcmp(c.v.Operator.val, ")") ||
                         !strcmp(c.v.Operator.val, "}") ||
                         !strcmp(c.v.Operator.val, "]"))
                    gb--;

                if (!strcmp(c.v.Operator.val, ",") && !gb)
                {
                    var_t _Gv = _PSF_GenerateVarFromByte(args);

                    if (result.v.function_decl.arg_size)
                        result.v.function_decl.args = OSF_Realloc(result.v.function_decl.args, (result.v.function_decl.arg_size + 1) * sizeof(var_t));

                    result.v.function_decl.args[result.v.function_decl.arg_size++] = _Gv;

                    OSF_Free(args->nodes);
                    OSF_Free(args);
                    args = _PSF_newByteArray();
                }
            }

            if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
                break;

            PSF_AST_ByteArray_AddNode(args, c);
            idx++;
        }

        idx++; // Eat ')'
    }

    body = _PSF_GetBody(arr, idx, _PSF_GetTabspace(arr, idx - 1));
    mod_t *body_mod = SF_CreateModule(MODULE_TYPE_FILE, body);
    SF_FrameIT_fromAST(body_mod);

    result.v.function_decl.body = BODY(body_mod)->body;
    result.v.function_decl.body_size = BODY(body_mod)->body_size;
    result.v.function_decl.name = NULL;

    if (name->size)
    {
        if (name->size == 1)
        {
            if (name->nodes[0].nval_type == AST_NVAL_TYPE_IDENTIFIER)
            {
                result.v.function_decl.name = strdup(name->nodes[0].v.Identifier.val);
            }
        }
    }

    OSF_Free(body_mod->var_holds);
    OSF_Free(body_mod);

    if (bd_sz_ptr != NULL)
        *bd_sz_ptr = body->size;

    OSF_Free(name->nodes);
    OSF_Free(name);
    OSF_Free(args->nodes);
    OSF_Free(args);
    OSF_Free(body->nodes);
    OSF_Free(body);

    return result;
}

stmt_t _PSF_ConstructClassStmt(psf_byte_array_t *arr, int idx, size_t *res_loc_ptr)
{
    stmt_t ret;
    ret.type = STATEMENT_TYPE_CLASS_DECL;
    ret.v.class_decl.name = NULL;
    psf_byte_array_t *name = _PSF_newByteArray(),
                     *body;
    int gb = 0;

    while (idx < arr->size)
    {
        psf_byte_t c = arr->nodes[idx];

        if (c.nval_type == AST_NVAL_TYPE_OPERATOR)
        {
            if (!strcmp(c.v.Operator.val, "(") ||
                !strcmp(c.v.Operator.val, "{") ||
                !strcmp(c.v.Operator.val, "["))
                gb++;
            else if (!strcmp(c.v.Operator.val, ")") ||
                     !strcmp(c.v.Operator.val, "}") ||
                     !strcmp(c.v.Operator.val, "]"))
                gb--;
        }

        if (c.nval_type == AST_NVAL_TYPE_NEWLINE && !gb)
            break;

        PSF_AST_ByteArray_AddNode(name, c);
        idx++;
    }

    if (name->size == 1)
    {
        if (name->nodes[0].nval_type == AST_NVAL_TYPE_IDENTIFIER)
        {
            ret.v.class_decl.name = strdup(name->nodes[0].v.Identifier.val);
        }
    }

    body = _PSF_GetBody(arr, idx, _PSF_GetTabspace(arr, idx - 1));

    mod_t *body_mod = SF_CreateModule(MODULE_TYPE_CLASS, body);
    SF_FrameIT_fromAST(body_mod);

    ret.v.class_decl.body = BODY(body_mod)->body;
    ret.v.class_decl.body_size = BODY(body_mod)->body_size;

    if (res_loc_ptr != NULL)
        *res_loc_ptr = idx + body->size;

    OSF_Free(body_mod->var_holds);
    OSF_Free(body_mod);
    OSF_Free(name->nodes);
    OSF_Free(name);
    OSF_Free(body->nodes);
    OSF_Free(body);

    return ret;
}

class_t **PSG_GetClasses(void)
{
    return &PSG_Classes;
}

int *PSG_GetClassesSize(void)
{
    return &PSG_Classes_Size;
}

int PSG_AddClass(class_t _class)
{
    if (*PSG_GetClassesSize())
        *PSG_GetClasses() = OSF_Realloc(*PSG_GetClasses(), ((*PSG_GetClassesSize()) + 1) * sizeof(class_t));

    (*PSG_GetClasses())[(*PSG_GetClassesSize())] = _class;

    return (*PSG_GetClassesSize())++;
}

int PSG_AddClassObject_toModule(class_t _class, mod_t *mod)
{
    if (!mod->class_objects_count)
    {
        mod->class_objects = (class_t *)OSF_Malloc(sizeof(class_t));
        ((class_t *)mod->class_objects)[mod->class_objects_count++] = _class;
        return 0;
    }

    mod->class_objects = OSF_Realloc(mod->class_objects, (mod->class_objects_count + 1) * sizeof(class_t));
    ((class_t *)mod->class_objects)[mod->class_objects_count++] = _class;

    return mod->class_objects_count - 1;
}

int_tuple PSG_AddClassObject_toClass(class_t _class, int idx)
{
    class_t *_cref = &((*PSG_GetClasses())[idx]);

    if (_cref->object_count)
        _cref->objects = OSF_Realloc(_cref->objects, (_cref->object_count + 1) * sizeof(class_t));

    _cref->objects[_cref->object_count++] = _class;

    return (int_tuple){.x1 = idx, .x2 = _cref->object_count - 1};
}