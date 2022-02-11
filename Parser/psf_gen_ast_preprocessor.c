#include "psf_gen_ast_preprocessor.h"

const char *PSF_ValidTokens[] = {
    "for",
    "if",
    "else",
    "while",
    "do",
    "break",
    "continue",
    "return",
    "to",
    "step",
    "in",
    "then",
    "with",
    "repeat",
    "fun",
    "class",
    "import",
    "as",
    "switch",
    "case",
    "default",
    "assert",
    "from",
    "and",
    "or",
    "not",
    "try",
    "except",
    NULL};

const char *PSF_ValidDataTypes[] = {
    "None",
    NULL};

const char *PSF_ValidBooleans[] = {
    "True",
    "False",
    NULL};

void PSF_AST_Preprocess_fromByteArray(psf_byte_array_t *_AST)
{
    int lincr = 1;
    for (size_t i = 0; i < _AST->size; i++)
    {
        _AST->nodes[i].line = lincr;

        if (_AST->nodes[i].nval_type == AST_NVAL_TYPE_NEWLINE)
            lincr++;

        if (_AST->nodes[i].nval_type == AST_NVAL_TYPE_IDENTIFIER)
        {
            int j = 0;
            while (PSF_ValidTokens[j] != NULL)
                if (!strcmp(_AST->nodes[i].v.Identifier.val, PSF_ValidTokens[j++]))
                    _AST->nodes[i].v.Identifier.is_token = 1;

            j = 0;
            while (PSF_ValidDataTypes[j] != NULL)
                if (!strcmp(_AST->nodes[i].v.Identifier.val, PSF_ValidDataTypes[j++]))
                {
                    char *ccpy = _AST->nodes[i].v.Identifier.val;
                    _AST->nodes[i].v.DataType.val = (char *)OSF_strdup(ccpy);
                    OSF_Free(ccpy);
                    _AST->nodes[i].nval_type = AST_NVAL_TYPE_DATA_TYPE;
                }

            j = 0;
            while (PSF_ValidBooleans[j] != NULL)
                if (!strcmp(_AST->nodes[i].v.Identifier.val, PSF_ValidBooleans[j++]))
                {
                    _AST->nodes[i].v.Bool.val = !strcmp(_AST->nodes[i].v.Identifier.val, "True") ? 1 : 0;
                    _AST->nodes[i].nval_type = AST_NVAL_TYPE_BOOL;
                }
        }
    }
}