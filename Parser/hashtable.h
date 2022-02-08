#pragma once

#include <stdio.h>
#include <string.h>

struct _hash_table_s
{
    char *c;
    struct _hash_table_s *members;
    int mem_count;
    int end;
    void *data;
};

typedef struct _hash_table_s HashTable;

HashTable Hash_new(char *, HashTable *, int, int);
HashTable *Hash_new_ptr(char *, HashTable *, int, int);
void Hash_add(HashTable *, char *, void *);
void *Hash_get(HashTable *, char *);