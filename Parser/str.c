#include "str.h"

char **Sf_Str_Split(char *str, char *delim)
{
    if (strstr(str, delim) == NULL) // At least one occurrance
        return NULL;

    char **arr = OSF_Malloc(sizeof(char *));
    int ac = 0, lid = 0;
    int str_len = strlen(str),
        delim_len = strlen(delim);

    for (size_t i = 0; i < str_len; i++)
    {
        char *s = OSF_Malloc((delim_len + 1) * sizeof(char));
        *s = '\0';

        for (size_t j = 0; j < delim_len && j < str_len; j++)
            strcat(s, (char[]){str[i + j], '\0'});

        if (!strcmp(s, delim))
        {
            if (ac)
                arr = OSF_Realloc(arr, (ac + 1) * sizeof(char *));

            arr[ac] = OSF_Malloc((i + 1 - lid) * sizeof(char));

            for (size_t j = lid; j < i; j++)
                arr[ac][j - lid] = str[j];

            arr[ac][i] = '\0';

            lid = i + 1;
            ac++;
        }
    }

    if (!strcmp(str + str_len - delim_len, delim))
    {
        if (ac)
            arr = OSF_Realloc(arr, (ac + 1) * sizeof(char *));

        arr[ac] = OSF_Malloc(sizeof(char));
        *(arr[ac]) = '\0';
        str_len -= delim_len;
        ac++;
    }
    else
    {
        if (lid != str_len)
        {
            int i = str_len;
            if (ac)
                arr = OSF_Realloc(arr, (ac + 1) * sizeof(char *));

            arr[ac] = OSF_Malloc((i + 1 - lid) * sizeof(char));

            for (size_t j = lid; j < i; j++)
                arr[ac][j - lid] = str[j];

            arr[ac][i] = '\0';
            ac++;
        }
    }

    if (ac)
        arr = OSF_Realloc(arr, (ac + 1) * sizeof(char *));
    arr[ac++] = NULL;

    return arr;
}