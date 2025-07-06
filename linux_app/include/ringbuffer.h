
#ifndef _RINGBUFFER_H__
#define _RINGBUFFER_H__

#include "public.h"

/* ring buffer */
struct ringbuffer_t {
    unsigned char *buffer_ptr;
    unsigned short read_mirror : 1;
    unsigned short read_index : 15;
    CACHELINE_PADDING(_pad_);
    unsigned short write_mirror : 1;
    unsigned short write_index : 15;
    short buffer_size;
};

enum ringbuffer_state_t {
    RINGBUFFER_EMPTY,
    RINGBUFFER_FULL,
    RINGBUFFER_HALFFULL,
};

void ringbuffer_init(struct ringbuffer_t *rb, unsigned char *pool, short size);
void ringbuffer_reset(struct ringbuffer_t *rb);
unsigned int  ringbuffer_put(struct ringbuffer_t *rb, const unsigned char *ptr, unsigned short length);
unsigned int  ringbuffer_put_force(struct ringbuffer_t *rb, const unsigned char *ptr, unsigned short length);
unsigned int  ringbuffer_putchar(struct ringbuffer_t *rb, const unsigned char ch);
unsigned int  ringbuffer_putchar_force(struct ringbuffer_t *rb, const unsigned char ch);
unsigned int  ringbuffer_get(struct ringbuffer_t *rb, unsigned char *ptr, unsigned short length);
unsigned int  ringbuffer_getchar(struct ringbuffer_t *rb, unsigned char *ch);
unsigned int  ringbuffer_data_len(struct ringbuffer_t *rb);
struct ringbuffer_t *ringbuffer_create(unsigned short length);
void ringbuffer_destroy(struct ringbuffer_t *rb);

#endif
