
#include "queue.h"
#include "public.h"

void queue_init(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);

    list_init(&pq->list);
    memset(&pq->qdat, 0, sizeof(qdata_t));
}

void queue_destory(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);

    list_t *pos;
    list_t *head = &pq->list;

    list_for_each(pos, head) 
    {
        list_remove(pos);
        free(list_entry(pos, queue_t, list));
    }

    queue_init(pq);
}

void queue_push(queue_t *pq, qdata_t *data)
{
    Imx_AssertVoid(pq != NULL);

    queue_t *q = (queue_t *)malloc(sizeof(queue_t));
    Imx_AssertVoid(q);

    memcpy(&q->qdat, data, sizeof(qdata_t));

    list_insert_before(&pq->list, &q->list);
}

void queue_push_urgent(queue_t *pq, qdata_t *data)
{
    Imx_AssertVoid(pq != NULL);

    queue_t *q = (queue_t *)malloc(sizeof(queue_t));
    Imx_AssertVoid(q);

    memcpy(&q->qdat, data, sizeof(qdata_t));

    list_insert_after(&pq->list, &q->list);
}

void queue_pop(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);
    Imx_AssertVoid(!queue_empty(pq));

    list_t *head = &pq->list;
    list_t *pos = head->next;

    list_remove(pos);
    free(list_entry(pos, queue_t, list));
}

qdata_t *queue_front(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);
    Imx_AssertVoid(!queue_empty(pq));

    list_t *pos = pq->list.next;
    queue_t *q = list_entry(pos, queue_t, list);

    return &q->qdat;
}

qdata_t *queue_back(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);
    Imx_AssertVoid(!queue_empty(pq));

    list_t *pos = pq->list.prev;
    queue_t *q = list_entry(pos, queue_t, list);

    return &q->qdat;
}

bool queue_empty(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);

    return (list_isempty(&pq->list));
}

unsigned int queue_size(queue_t *pq)
{
    Imx_AssertVoid(pq != NULL);
    Imx_AssertVoid(!queue_empty(pq));

    return (list_len(&pq->list));
}

#if 1  //test demo
void queue_test(void)
{
    qdata_t qdat1, qdat2, qdat3, qdat4;
    queue_t *q = (queue_t *)malloc(sizeof(queue_t));

    queue_init(q);

    qdat1.size = 2;
    qdat1.data = malloc(qdat1.size);
    memset(qdat1.data, 1, qdat1.size);
    queue_push(q, &qdat1);

    qdat2.size = 2;
    qdat2.data = malloc(qdat2.size);
    memset(qdat2.data, 2, qdat2.size);
    queue_push(q, &qdat2);

    qdat3.size = 2;
    qdat3.data = malloc(qdat3.size);
    memset(qdat3.data, 3, qdat3.size);
    queue_push(q, &qdat3);

    qdat4.size = 2;
    qdat4.data = malloc(qdat4.size);
    memset(qdat4.data, 4, qdat4.size);
    queue_push(q, &qdat4);

    while (!queue_empty(q)) {
        unsigned int size = queue_size(q);
        qdata_t *front = queue_front(q);
        qdata_t *back = queue_back(q);

        printf("qsize = %d: front.size = %d, front.data = %d; back.size  = %d, back.data  = %d\n",
               size, front->size, ((char *)(front->data))[0], back->size, ((char *)(back->data))[0]);

        queue_pop(q);
    }

    free(qdat4.data);
    free(qdat3.data);
    free(qdat2.data);
    free(qdat1.data);
    free(q);
}
#endif
