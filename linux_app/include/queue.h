
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "list.h"
#include "public.h"

struct queue_data_t {
    unsigned int type;
    void *data;
    unsigned int size;
};
typedef struct queue_data_t qdata_t;

struct queue_node_t {
    list_t list;
    qdata_t qdat;
};
typedef struct queue_node_t queue_t;

void queue_init(queue_t *pq);
void queue_destory(queue_t *pq);
void queue_push(queue_t *pq, qdata_t *data);
void queue_push_urgent(queue_t *pq, qdata_t *data);
void queue_pop(queue_t *pq);
qdata_t *queue_front(queue_t *pq);
qdata_t *queue_back(queue_t *pq);
bool queue_empty(queue_t *pq);
unsigned int queue_size(queue_t *pq);

#endif
