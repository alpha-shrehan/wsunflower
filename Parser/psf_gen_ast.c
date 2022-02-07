#include "psf_gen_ast.h"

void PSF_AST_ByteArray_AddNode(psf_byte_array_t *_Target, psf_byte_t _Node)
{
    if (_Target->size)
        _Target->nodes = (psf_byte_t *)OSF_Realloc(_Target->nodes, (_Target->size + 1) * sizeof(psf_byte_t));

    _Target->nodes[_Target->size] = _Node;
    _Target->size++;
}

psf_byte_array_t *PSF_AST_fromString(const char *src)
{
    psf_byte_array_t *ast;
    if (src == NULL)
    {
        ast = _PSF_newByteArray();
        return ast;
    }

    ast = _PSF_New_AST_FromString(src);

    return ast;
}

void PSF_AST_print(psf_byte_array_t *ast)
{
    const char *nv_re[] = {"AST_NVAL_TYPE_DATA_TYPE",
                           "AST_NVAL_TYPE_INT",
                           "AST_NVAL_TYPE_FLOAT",
                           "AST_NVAL_TYPE_STRING",
                           "AST_NVAL_TYPE_IDENTIFIER",
                           "AST_NVAL_TYPE_NEWLINE",
                           "AST_NVAL_TYPE_COMMENT",
                           "AST_NVAL_TYPE_TABSPACE",
                           "AST_NVAL_TYPE_OPERATOR",
                           "AST_NVAL_TYPE_BOOL"};

    for (size_t i = 0; i < ast->size; i++)
    {
        printf("%s ", nv_re[ast->nodes[i].nval_type]);

        switch (ast->nodes[i].nval_type)
        {
        case AST_NVAL_TYPE_TABSPACE:
            printf("%d\n", ast->nodes[i].v.Tabspace.len);
            break;
        case AST_NVAL_TYPE_STRING:
            printf("%s\n", ast->nodes[i].v.String.val);
            break;
        case AST_NVAL_TYPE_FLOAT:
            printf("%f\n", ast->nodes[i].v.Float.val);
            break;
        case AST_NVAL_TYPE_INT:
            printf("%d\n", ast->nodes[i].v.Int.val);
            break;
        case AST_NVAL_TYPE_IDENTIFIER:
            printf("(is_token: %d) %s\n", ast->nodes[i].v.Identifier.is_token, ast->nodes[i].v.Identifier.val);
            break;
        case AST_NVAL_TYPE_COMMENT:
            printf("%s\n", ast->nodes[i].v.Comment.cmt);
            break;
        case AST_NVAL_TYPE_DATA_TYPE:
            printf("%s\n", ast->nodes[i].v.DataType.val);
            break;
        case AST_NVAL_TYPE_OPERATOR:
            printf("%s\n", ast->nodes[i].v.Operator.val);
            break;
        default:
            printf("\n");
            break;
        }
    }
}

void PSF_AST_print_node(psf_byte_t ast)
{
    const char *nv_re[] = {"AST_NVAL_TYPE_DATA_TYPE",
                           "AST_NVAL_TYPE_INT",
                           "AST_NVAL_TYPE_FLOAT",
                           "AST_NVAL_TYPE_STRING",
                           "AST_NVAL_TYPE_IDENTIFIER",
                           "AST_NVAL_TYPE_NEWLINE",
                           "AST_NVAL_TYPE_COMMENT",
                           "AST_NVAL_TYPE_TABSPACE",
                           "AST_NVAL_TYPE_OPERATOR",
                           "AST_NVAL_TYPE_BOOL"};

    printf("%s ", nv_re[ast.nval_type]);

    switch (ast.nval_type)
    {
    case AST_NVAL_TYPE_TABSPACE:
        printf("%d\n", ast.v.Tabspace.len);
        break;
    case AST_NVAL_TYPE_STRING:
        printf("%s\n", ast.v.String.val);
        break;
    case AST_NVAL_TYPE_FLOAT:
        printf("%f\n", ast.v.Float.val);
        break;
    case AST_NVAL_TYPE_INT:
        printf("%d\n", ast.v.Int.val);
        break;
    case AST_NVAL_TYPE_IDENTIFIER:
        printf("(is_token: %d) %s\n", ast.v.Identifier.is_token, ast.v.Identifier.val);
        break;
    case AST_NVAL_TYPE_COMMENT:
        printf("%s\n", ast.v.Comment.cmt);
        break;
    case AST_NVAL_TYPE_DATA_TYPE:
        printf("%s\n", ast.v.DataType.val);
        break;
    case AST_NVAL_TYPE_OPERATOR:
        printf("%s\n", ast.v.Operator.val);
        break;
    default:
        printf("\n");
        break;
    }
}

psf_byte_array_t *_PSF_New_AST_FromString(const char *src)
{
    psf_byte_array_t *ast = (psf_byte_array_t *)OSF_Malloc(sizeof(psf_byte_array_t));
    ast->nodes = (psf_byte_t *)OSF_Malloc(sizeof(psf_byte_t));
    ast->size = 0;
    int src_len = strlen(src);

    for (size_t i = 0; i < src_len; i++)
    {
        char c = src[i];

        if (strstr("0123456789", (char[]){c, '\0'}) != NULL) // Number
        {
            char *num = (char *)OSF_Malloc(sizeof(char));
            int num_len = 0, is_float = 0;

            while (strstr("0123456789.", (char[]){c, '\0'}) != NULL)
            {
                if (c == '.')
                {
                    if (is_float || strstr("0123456789.", (char[]){src[i + 1], '\0'}) == NULL)
                        break;
                    is_float = 1;
                }
                num = (char *)OSF_Realloc(num, (num_len + 1) * sizeof(char));
                num[num_len] = c;
                num_len++;
                i++;
                c = src[i];
            }

            num = (char *)OSF_Realloc(num, (num_len + 1) * sizeof(char));
            num[num_len] = (c = '\0');
            num_len++;

            if (is_float)
                PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){.nval_type = AST_NVAL_TYPE_FLOAT, .v = {.Float.val = strtod(num, NULL)}});
            else
                PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){.nval_type = AST_NVAL_TYPE_INT, .v = {.Int.val = atoi(num)}});

            OSF_Free(num);
            num = NULL;

            i--;
            continue;
        }

        if (
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c == '_')) // identifier
        {
            char *identifier = (char *)OSF_Malloc(sizeof(char));
            int identifier_len = 0;

            while (
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                (c == '_') ||
                (c >= '0' && c <= '9'))
            {
                identifier = (char *)OSF_Realloc(identifier, (identifier_len + 1) * sizeof(char));
                identifier[identifier_len] = c;
                identifier_len++;
                i++;
                c = src[i];
            }

            identifier = (char *)OSF_Realloc(identifier, (identifier_len + 1) * sizeof(char));
            identifier[identifier_len] = (c = '\0');
            identifier_len++;

            PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){.nval_type = AST_NVAL_TYPE_IDENTIFIER, .v = {.Identifier.val = (char *)OSF_strdup(identifier), .Identifier.is_token = 0}});

            OSF_Free(identifier);
            identifier = NULL;

            i--;
            continue;
        }

        if (strstr("+-*/%^=<>!&|~:;,.()[]{}@", (char[]){c, '\0'}) != NULL) // Operator
        {
            char op1 = 0, op2 = 0, op3 = 0;

            op1 = c;
            if (i + 1 < src_len)
            {
                op2 = src[i + 1];
                if (i + 2 < src_len)
                    op3 = src[i + 2];
            }

            char *res_op = _PSF_Construct_Operator_fromString(op1, op2, op3);

            PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){.nval_type = AST_NVAL_TYPE_OPERATOR, .v = {.Operator.val = OSF_strdup(res_op)}});

            i += strlen(res_op) - 1;
            OSF_Free(res_op);
            res_op = NULL;
            continue;
        }

        switch (c)
        {
        case '\n':
            PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){
                                               .nval_type = AST_NVAL_TYPE_NEWLINE});
            break;
        case ' ':
        {
            if (!i)
                continue;

            if (ast->size)
            {
                if (src[i - 1] == ' ' && ast->nodes[ast->size - 1].nval_type == AST_NVAL_TYPE_NEWLINE)
                {
                    PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){
                                                       .nval_type = AST_NVAL_TYPE_TABSPACE,
                                                       .v.Tabspace = {.len = 2}});
                    continue;
                }

                if (ast->nodes[ast->size - 1].nval_type == AST_NVAL_TYPE_TABSPACE)
                {
                    ast->nodes[ast->size - 1].v.Tabspace.len++;
                }
            }
        }
        break;
        case '\t':
        {
            if (ast->size)
            {
                if (ast->nodes[ast->size - 1].nval_type == AST_NVAL_TYPE_NEWLINE)
                {
                    PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){
                                                       .nval_type = AST_NVAL_TYPE_TABSPACE,
                                                       .v.Tabspace = {.len = 4}});
                    continue;
                }

                if (ast->nodes[ast->size - 1].nval_type == AST_NVAL_TYPE_TABSPACE)
                {
                    ast->nodes[ast->size - 1].v.Tabspace.len += 4;
                }
            }
            else
            {
                PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){
                                                   .nval_type = AST_NVAL_TYPE_TABSPACE,
                                                   .v.Tabspace = {.len = 4}});
            }
        }
        break;
        case '#':
        {
            char *cmt = (char *)OSF_Malloc(sizeof(char));
            int cmt_len = 0;

            while (c != '\n')
            {
                cmt[cmt_len] = c;
                cmt_len++;
                cmt = (char *)OSF_Realloc(cmt, (cmt_len + 1) * sizeof(char));
                i++;
                c = src[i];
            }

            cmt = (char *)OSF_Realloc(cmt, (cmt_len + 1) * sizeof(char));
            cmt[cmt_len] = (c = '\0');
            cmt_len++;

            PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){
                                               .nval_type = AST_NVAL_TYPE_COMMENT,
                                               .v.Comment = {.cmt = (char *)cmt}});
            OSF_Free(cmt);
            cmt = NULL;

            i--;
        }
        break;
        case '\'':
        case '\"':
        {
            char *str = (char *)OSF_Malloc(sizeof(char));
            int str_len = 1;
            char pres_c = c;

            *str = '\'';
            i++;

            for (; src[i] != pres_c; i++)
            {
                str = (char *)OSF_Realloc(str, (str_len + 1) * sizeof(char));
                str[str_len] = src[i];

                if (str[str_len] == '\\')
                {
                    char next_c = src[i + 1];

                    switch (next_c)
                    {
                    case 'n':
                        str[str_len] = '\n';
                        break;
                    case 't':
                        str[str_len] = '\t';
                        break;
                    case 'r':
                        str[str_len] = '\r';
                        break;
                    case '\\':
                        str[str_len] = '\\';
                        break;
                    case '\'':
                    case '\"':
                        str[str_len] = next_c;
                        break;
                    default:
                        break;
                    }
                    i++;
                }

                str_len++;
            }

            str = (char *)OSF_Realloc(str, (str_len + 2) * sizeof(char));
            str[str_len] = '\'';
            str_len++;
            str[str_len] = (c = '\0');
            str_len++;

            PSF_AST_ByteArray_AddNode(ast, (psf_byte_t){
                                               .nval_type = AST_NVAL_TYPE_STRING,
                                               .v.String = {.val = (char *)OSF_strdup(str)}});

            OSF_Free(str);
            str = NULL;
        }
        break;

        default:
            break;
        }
    }

    return ast;
}

char *_PSF_Construct_Operator_fromString(char op1, char op2, char op3)
{
    char *res = OSF_Malloc(sizeof(char) * 4);
    res[0] = '\0';
    res[1] = '\0';
    res[2] = '\0';
    res[3] = '\0';

    switch (op1)
    {
    case '+':
    case '-':
    case '/':
    case '%':
    case '^':
    case '=':
    case '!':
    case '&':
    case '|':
    case '~':
    {
        res[0] = op1;

        switch (op2)
        {
        case '=':
        {
            res[1] = op2;
            break;
        }
        default:
            break;
        }
    }
    break;
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
    case ',':
    case ';':
    case '.':
    case ':':
    case '@':
    {
        res[0] = op1;
    }
    break;
    case '<':
    case '>':
    {
        res[0] = op1;

        if (op1 == op2 || op2 == '=')
            res[1] = op2;

        if (op3 == '=')
            res[2] = op3;
    }
    break;
    case '*':
    {
        res[0] = op1;

        if (op2 == '=')
            res[1] = op2;

        if (op2 == op1)
        {
            res[1] = op2;

            if (op3 == '=')
                res[2] = op3;
        }
    }
    break;
    default:
        break;
    }

    char *rr = OSF_strdup(res);
    OSF_Free(res);
    return rr;
}

psf_byte_array_t *_PSF_newByteArray(void)
{
    psf_byte_array_t *_new = (psf_byte_array_t *)OSF_Malloc(sizeof(psf_byte_t));

    _new->nodes = (psf_byte_t *)OSF_Malloc(sizeof(psf_byte_t));
    _new->size = 0;

    return _new;
}

char *_PSF_TrimSFStrImp(char *_str)
{
    char *_ccpy = OSF_strdup(_str);
    _ccpy++;
    _ccpy[strlen(_ccpy) - 1] = '\0';

    return _ccpy;
}