#include "sunflower.h"

int main(int argc, char const *argv[])
{
    SF_InitEnv();

    Trie *ht = Trie_new_ptr("", NULL, 0, 0);

    Trie_add(ht, "test", (int *)(int []){10});
    Trie_add(ht, "testing", (int *)(int []){20});
    Trie_add(ht, "tests", (int *)(int[]){30});
    Trie_add(ht, "hello", (int *)(int[]){40});
    Trie_add(ht, "hell", (int *)(int[]){50});
    Trie_add(ht, "test", (int *)(int[]){60});

    printf("%d\n", *(int *)Trie_get(ht, "testing"));

    SF_DestroyEnv();
    return 0;
}