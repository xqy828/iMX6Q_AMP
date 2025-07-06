
#include "ringbuffer.h"
#include "public.h"

unsigned short ringbuffer_get_size(struct ringbuffer_t *rb)
{
    Imx_AssertVoid(rb != NULL);

    return rb->buffer_size;
}

unsigned int ringbuffer_space_len(struct ringbuffer_t *rb)
{
    Imx_AssertVoid(rb != NULL);

    return ((rb)->buffer_size - ringbuffer_data_len(rb));
}

enum ringbuffer_state_t ringbuffer_status(struct ringbuffer_t *rb)
{
    if (rb->read_index == rb->write_index) 
    {
        if (rb->read_mirror == rb->write_mirror) 
        {
            return RINGBUFFER_EMPTY;
        } else 
        {
            return RINGBUFFER_FULL;
        }
    }

    return RINGBUFFER_HALFFULL;
}

void ringbuffer_init(struct ringbuffer_t *rb,
                     unsigned char *pool,
                     short size)
{
    Imx_AssertVoid(rb != NULL);
    Imx_AssertVoid(size > 0);
    Imx_AssertVoid((size & 0x03) == 0);

    /* initialize read and write index */
    rb->read_mirror = rb->read_index = 0;
    rb->write_mirror = rb->write_index = 0;

    /* set buffer pool and size */
    rb->buffer_ptr = pool;
    rb->buffer_size = size;
}

/**
 * put a block of data into ring buffer
 */
unsigned int ringbuffer_put(struct ringbuffer_t *rb,
                        const unsigned char *ptr,
                        unsigned short length)
{
    unsigned short size;

    Imx_AssertVoid(rb != NULL);

    /* whether has enough space */
    size = ringbuffer_space_len(rb);

    /* no space */
    if (size == 0) 
    {
        return 0;
    }

    /* drop some data */
    if (size < length) 
    {
        length = size;
    }

    if (rb->buffer_size - rb->write_index > length) 
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;

        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    return length;
}

/**
 * put a block of data into ring buffer
 *
 * When the buffer is full, it will discard the old data.
 */
unsigned int ringbuffer_put_force(struct ringbuffer_t *rb,
                              const unsigned char *ptr,
                              unsigned short length)
{
    unsigned short space_length;

    Imx_AssertVoid(rb != NULL);

    space_length = ringbuffer_space_len(rb);

    if (length > rb->buffer_size) 
    {
        ptr = &ptr[length - rb->buffer_size];
        length = rb->buffer_size;
    }

    if (rb->buffer_size - rb->write_index > length) 
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;

        if (length > space_length) 
        {
            rb->read_index = rb->write_index;
        }

        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           length - (rb->buffer_size - rb->write_index));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    if (length > space_length) 
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = rb->write_index;
    }

    return length;
}

/**
 *  get data from ring buffer
 */
unsigned int ringbuffer_get(struct ringbuffer_t *rb,
                        unsigned char *ptr,
                        unsigned short length)
{
    unsigned int size;

    Imx_AssertVoid(rb != NULL);

    /* whether has enough data  */
    size = ringbuffer_data_len(rb);

    /* no data */
    if (size == 0) 
    {
        return 0;
    }

    /* less data */
    if (size < length) 
    {
        length = size;
    }

    if (rb->buffer_size - rb->read_index > length) 
    {
        /* copy all of data */
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->read_index += length;

        return length;
    }

    memcpy(&ptr[0],
           &rb->buffer_ptr[rb->read_index],
           rb->buffer_size - rb->read_index);
    memcpy(&ptr[rb->buffer_size - rb->read_index],
           &rb->buffer_ptr[0],
           length - (rb->buffer_size - rb->read_index));

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buffer_size - rb->read_index);

    return length;
}

/**
 * put a character into ring buffer
 */
unsigned int ringbuffer_putchar(struct ringbuffer_t *rb, const unsigned char ch)
{
    Imx_AssertVoid(rb != NULL);

    /* whether has enough space */
    if (!ringbuffer_space_len(rb)) 
    {
        return 0;
    }

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size - 1) 
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    } else 
    {
        rb->write_index++;
    }

    return 1;
}

/**
 * put a character into ring buffer
 *
 * When the buffer is full, it will discard one old data.
 */
unsigned int ringbuffer_putchar_force(struct ringbuffer_t *rb, const unsigned char ch)
{
    enum ringbuffer_state_t old_state;

    Imx_AssertVoid(rb != NULL);

    old_state = ringbuffer_status(rb);

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size - 1) 
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == RINGBUFFER_FULL) 
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    } else 
    {
        rb->write_index++;
        if (old_state == RINGBUFFER_FULL) 
        {
            rb->read_index = rb->write_index;
        }
    }

    return 1;
}

/**
 * get a character from a ringbuffer
 */
unsigned int ringbuffer_getchar(struct ringbuffer_t *rb, unsigned char *ch)
{
    Imx_AssertVoid(rb != NULL);

    /* ringbuffer is empty */
    if (!ringbuffer_data_len(rb)) 
    {
        return 0;
    }

    /* put character */
    *ch = rb->buffer_ptr[rb->read_index];

    if (rb->read_index == rb->buffer_size - 1) 
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    } else 
    {
        rb->read_index++;
    }

    return 1;
}

/**
 * get the size of data in rb
 */
unsigned int ringbuffer_data_len(struct ringbuffer_t *rb)
{
    switch (ringbuffer_status(rb)) 
    {
        case RINGBUFFER_EMPTY:
        return 0;
        case RINGBUFFER_FULL:
        return rb->buffer_size;
        case RINGBUFFER_HALFFULL:
        default:
            if (rb->write_index > rb->read_index) 
            {
                return rb->write_index - rb->read_index;
            } 
            else 
            {
                return rb->buffer_size - (rb->read_index - rb->write_index);
            }
    };
}

/**
 * empty the rb
 */
void ringbuffer_reset(struct ringbuffer_t *rb)
{
    Imx_AssertVoid(rb != NULL);

    rb->read_mirror = 0;
    rb->read_index = 0;
    rb->write_mirror = 0;
    rb->write_index = 0;
}

struct ringbuffer_t *ringbuffer_create(unsigned short size)
{
    struct ringbuffer_t *rb;
    unsigned char *pool;

    Imx_AssertVoid(size > 0);    
    Imx_AssertVoid((size & 0x03) == 0);

    rb = calloc(1,sizeof(struct ringbuffer_t));
    if (rb == NULL) 
    {
        goto exit;
    }

    pool = calloc(1,size);
    if (pool == NULL) 
    {
        free(rb);
        rb = NULL;
        goto exit;
    }
    ringbuffer_init(rb, pool, size);

exit:

    return rb;
}

void ringbuffer_destroy(struct ringbuffer_t *rb)
{
    Imx_AssertVoid(rb != NULL);

    free(rb->buffer_ptr);
    free(rb);
}
