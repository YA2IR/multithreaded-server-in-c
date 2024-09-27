#include "hash_table.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 *   this is a simple hash table, it's quick-to-build, which means it's __not__ the most efficient
 */

hash_table_t* init_table(char** routes, int num_routes) {
    hash_table_t* table = malloc(sizeof(hash_table_t));

    table->num_keys = num_routes;

    // a slightly disturbing tradeoff, this value is used as a mod in the hash function
    while ((table->num_keys % 7 != 0) || (table->num_keys % 2 == 0)) {
        table->num_keys++;
    }

    table->nodes = malloc(sizeof(table_node*) * table->num_keys);

    for (int i = 0; i < table->num_keys; i++) {
        table->nodes[i] = NULL;
    }

    int hash_val;
    for (int i = 0; i < num_routes; i++) {
        table_node* new_node = malloc(sizeof(table_node));
        new_node->val = strdup(routes[i]);
        new_node->key = hash(table, routes[i]);
        new_node->next = NULL;

        hash_val = new_node->key;

        if (table->nodes[hash_val] == NULL) {
            table->nodes[hash_val] = new_node;
        } else {
            new_node->next = table->nodes[hash_val];
            table->nodes[hash_val] = new_node;
        }
    }

    return table;
}

unsigned int hash(hash_table_t* table, char* key) {
    unsigned long int hashval = 1618033 + 314159;
    char* original_key = key;

    while (*key) {
        hashval = (hashval << 5) + *key++;
    }
    return hashval % table->num_keys;
}

bool exists(hash_table_t* table, char* key) {
    int val = hash(table, key);
    table_node* node = table->nodes[val];

    while (node != NULL) {
        if (strcmp(node->val, key) == 0) {
            return true;
        }
        node = node->next;
    }
    return false;
}

void free_table(hash_table_t* table) {
    table_node* node;
    table_node* tmp;

    for (int i = 0; i < table->num_keys; i++) {
        node = table->nodes[i];
        while (node != NULL) {
            table_node* tmp = node;
            node = node->next;
            free(tmp->val);
            free(tmp);
        }
    }
    free(table->nodes);
    free(table);
}
