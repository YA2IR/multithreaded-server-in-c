
#include <pthread.h>

#define EMPTY -1

typedef struct node {
    int cd;
    struct node* next;
} node;

typedef struct {
    node* head;
    node* tail;
} queue_t;

queue_t* init_queue();
void enqueue(queue_t* q, int cd);
int dequeue(queue_t* q);

