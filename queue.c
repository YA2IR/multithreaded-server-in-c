#include "queue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

queue_t* init_queue() {
    queue_t* q = malloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    return q;
}

void enqueue(queue_t* q, int cd) {
    node* new_node = malloc(sizeof(node));
    new_node->cd = cd;
    new_node->next = NULL;

    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
}

int dequeue(queue_t* q) {
    if (q->head == NULL) return EMPTY;

    int tmp = q->head->cd;
    node* tmp_node = q->head;

    q->head = q->head->next;
    if (q->head == NULL) q->tail = NULL;

    free(tmp_node);

    return tmp;
}
