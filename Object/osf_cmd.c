#include "osf_cmd.h"

char **cmd_reprs[] = {
    (char *[]){"-h", "--help", NULL},
    (char *[]){"--version", NULL},
    (char *[]){"-v", "--verbose", NULL},
    (char *[]){"-E", "--err", NULL},
    NULL
};

int cmd_flag_reqs_val[] = {0, 0, 0, 0, -1};

static clopt_t *cmd_flags;
static int cmd_flags_count;

void OSF_cmd_init(void)
{
    cmd_flags = OSF_Malloc(sizeof(clopt_t));
    cmd_flags_count = 0;
}

int OSF_cmd_is_valid_flag(char *flag)
{
    int i = 0;
    while (cmd_reprs[i] != NULL)
    {
        int j = 0;
        while (cmd_reprs[i][j] != NULL)
        {
            if (!strcmp(cmd_reprs[i][j], flag))
                return 1;
            j++;
        }

        i++;
    }

    return 0;
}

clopt_t **OSF_cmd_get_flags(void)
{
    return &cmd_flags;
}

int *OSF_cmd_get_flags_size(void)
{
    return &cmd_flags_count;
}

clopt_t OSF_cmd_flag_new(int fType, char *fValue, int fTv)
{
    if (cmd_flag_reqs_val[fType])
        if (fValue == NULL)
            exit(printf("Flag '%s' requires a value.\n", cmd_reprs[fType][0]));
        else
            ;
    else
        if (fValue != NULL)
            exit(printf("Flag '%s' does not require a value.\n", cmd_reprs[fType][0]));
    
    clopt_t v = {
        .type = fType,
        .val = fValue,
        .takes_val = fTv
    };

    return v;
}

void OSF_cmd_set_flag(clopt_t cl)
{
    if (*OSF_cmd_get_flags_size())
        *OSF_cmd_get_flags() = OSF_Realloc(*OSF_cmd_get_flags(), ((*OSF_cmd_get_flags_size()) + 1) * sizeof(clopt_t));
    
    (*OSF_cmd_get_flags())[(*OSF_cmd_get_flags_size())++] = cl;
}

clopt_t *OSF_cmd_get_flag_fromVal(char *val)
{
    int i = 0;
    while (i < *OSF_cmd_get_flags_size())
    {
        clopt_t c = (*OSF_cmd_get_flags())[i];
        if (c.takes_val)
        {
            int j = 0;
            while (cmd_reprs[j] != NULL)
            {
                int k = 0;
                while (cmd_reprs[j][k] != NULL)
                {
                    if (!strcmp(cmd_reprs[j][k], val))
                        return &((*OSF_cmd_get_flags())[i]);
                    k++;
                }
                j++;
            }
        }
        i++;
    }

    return NULL;
}

clopt_t *OSF_cmd_get_flag_fromType(int type)
{
    int i = 0;
    while (i < *OSF_cmd_get_flags_size())
    {
        clopt_t c = (*OSF_cmd_get_flags())[i];
        if (c.type == type)
            return &((*OSF_cmd_get_flags())[i]);
        
        i++;
    }

    return NULL;
}