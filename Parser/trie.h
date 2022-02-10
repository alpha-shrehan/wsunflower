#pragma once

#include <stdio.h>
#include <string.h>

struct _trie_s
{
    char *c;
    struct _trie_s *members;
    int mem_count;
    int end;
    void *data;
};

typedef struct _trie_s Trie;

Trie Trie_new(char *, Trie *, int, int);
Trie *Trie_new_ptr(char *, Trie *, int, int);
void Trie_add(Trie *, char *, void *);
void *Trie_get(Trie *, char *);