#include "hashtable.h"
#include <Object/osf_mem.h>

HashTable Hash_new(char *c, HashTable *mems, int mcount, int end)
{
    HashTable x = {
        .c = OSF_strdup(c != NULL ? c : ""),
        .end = end,
        .mem_count = mcount,
        .members = mems,
        .data = NULL};

    if (x.members == NULL)
        x.members = OSF_Malloc(sizeof(HashTable));

    return x;
}

HashTable *Hash_new_ptr(char *c, HashTable *mems, int mcount, int end)
{
    HashTable *x = OSF_Malloc(sizeof(HashTable));
    *x = Hash_new(c, mems, mcount, end);

    return x;
}

void Hash_add(HashTable *ht, char *nm, void *dat)
{
    HashTable *hc = ht;

    while (*nm != '\0')
    {
        char *_x = strstr(hc->c, (char *)(char[]){*nm, '\0'});

        if (_x == NULL)
        {
            if (hc->mem_count)
                hc->members = OSF_Realloc(hc->members, (hc->mem_count + 1) * sizeof(HashTable));

            hc->members[hc->mem_count] = Hash_new("", NULL, 0, 0);
            hc->mem_count++;

            hc->c = OSF_Realloc(hc->c, (strlen(hc->c) + 2) * sizeof(char));
            strcat(hc->c, (char *)(char[]){*nm, '\0'});

            hc = &(hc->members[hc->mem_count - 1]);
        }
        else
            hc = &(hc->members[_x - hc->c]);

        nm++;
    }

    char *_ss = strstr(hc->c, "$");

    if (_ss == NULL)
    {
        if (strlen(hc->c))
            hc->c = OSF_Realloc(hc->c, (strlen(hc->c) + 2) * sizeof(char));
        strcat(hc->c, "$");

        if (hc->mem_count)
            hc->members = OSF_Realloc(hc->members, (hc->mem_count + 1) * sizeof(HashTable));

        hc->members[hc->mem_count] = Hash_new("", NULL, 0, 1);
        hc->members[hc->mem_count].data = dat;
        hc->mem_count++;
    }
    else
    {
        int idx = _ss - hc->c;
        hc->members[idx] = Hash_new("", NULL, 0, 1);
        hc->members[idx].data = dat;
    }
}

void *Hash_get(HashTable *ht, char *str)
{
    HashTable *hc = ht;

    while (*str != '\0')
    {
        char *_x = strstr(hc->c, (char *)(char[]){*str, '\0'});
        if (_x == NULL)
            return NULL;

        hc = &(hc->members[_x - hc->c]);
        str++;
    }

    int ix = -1, i = 0, j = strlen(hc->c) - 1;

    while (i <= j)
    {
        if (hc->c[i] == '$')
        {
            ix = i;
            break;
        }

        if (hc->c[j] == '$')
        {
            ix = j;
            break;
        }

        i++;
        j--;
    }

    if (ix == -1)
        return NULL;
    return hc->members[ix].data;
}