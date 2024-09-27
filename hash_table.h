#include <stdbool.h>


struct table_node {
    int key;
    char* val;
    struct table_node* next;
} typedef table_node;

typedef struct {
    table_node** nodes;
    int num_keys;
} hash_table_t;

hash_table_t* init_table(char** keys, int num_values);
void free_table(hash_table_t* table);
unsigned int hash(hash_table_t* table, char* key);
bool exists(hash_table_t* table, char* key);
