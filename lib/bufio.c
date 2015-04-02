#include "bufio.h"
#include <stdio.h>

struct buf_t * buf_new(size_t capacity)
{
    struct buf_t * result = (struct buf_t*) malloc(sizeof(struct buf_t));
    if (!result)
        return NULL;
    result->capacity = capacity;
    result->size = 0;
    result->buffer = (char*) malloc(capacity);
    if (!result->buffer) {
        free(result);
        return NULL;
    }
    return result;
}

void buf_free(struct buf_t * buf)
{
    #ifdef DEBUG
    if (!buf)
        abort();
    #endif
    free(buf->buffer);
    free(buf);
}

size_t buf_capacity(struct buf_t * buf) 
{
    #ifdef DEBUG
    if (!buf)
        abort();
    #endif
    return buf->capacity;
}

size_t buf_size(struct buf_t * buf) 
{
    #ifdef DEBUG
    if (!buf)
        abort();
    #endif
    return buf->size;
}

ssize_t buf_fill(fd_t fd, struct buf_t * buf, size_t required)
{
    #ifdef DEBUG
    if (!buf || required > buf->capacity)
        abort();
    #endif
    ssize_t rc = 1;
    while (buf->size < required && (rc = read(fd, buf->buffer + buf->size, buf->capacity - buf->size)) >= 0) 
        buf->size += rc;
    return rc < 0 ? -1 : buf->size;
}

ssize_t buf_flush(fd_t fd, struct buf_t * buf, size_t required)
{
    #ifdef DEBUG
    if (!buf)
        abort();
    #endif
    size_t flushed = 0;
    ssize_t wc;
    while (flushed < required && flushed < buf->size && (wc = write(fd, buf->buffer, buf->size - flushed)) >= 0) 
        flushed += wc;
    buf->size -= flushed;
    for (int i = 0; i < buf->size; i++)
        buf->buffer[i] = buf->buffer[i + flushed];
    return wc < 0 ? -1 : 0;
}
