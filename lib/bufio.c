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
    if (!result->buffer) 
    {
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
    while (buf->size < required && (rc = read(fd, buf->buffer + buf->size, buf->capacity - buf->size)) > 0) 
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

ssize_t buf_getline(fd_t fd, struct buf_t * buf, char* dest)
{
    #ifdef DEBUG
    if (!buf)
        abort();
    #endif
    for (ssize_t i = 0; i < buf->size; i++)
        if (buf->buffer[i] == '\n') 
        {
            for (ssize_t k = 0; k < i; k++)
                dest[k] = buf->buffer[k];
            for (ssize_t k = i + 1, j = 0; k < buf->size; k++, j++)
                buf->buffer[j] = buf->buffer[k];
            buf->size -= i;
            return i;
        }
    buf->size = 0;
    while (1)
    {
        ssize_t last = buf->size;
        ssize_t rc = read(fd, buf->buffer + buf->size, buf->capacity - buf->size);
        if (rc == 0)
            break;
        if (rc < 0)
            return -1;
        buf->size += rc;
        for (ssize_t i = last; i < buf->size; i++)
            if (buf->buffer[i] == '\n') 
            {
                for (ssize_t k = 0; k < i; k++)
                    dest[k] = buf->buffer[k];
                for (ssize_t k = i + 1, j = 0; k < buf->size; k++, j++)
                    buf->buffer[j] = buf->buffer[k];
                buf->size -= i;
                return i;
            }
    }   

    return buf->size;    
}

ssize_t buf_write(fd_t fd, struct buf_t * buf, char* src, size_t len)
{
    #ifdef DEBUG
    if (!buf)
        abort();
    #endif
    for (int i = 0; i < len; i++)
    {
        if (buf->size == buf->capacity)
        {   
            ssize_t wc = write(fd, buf->buffer, buf->capacity);
            if (wc <= 0)
                return -1;
            for (int j = 0; j < buf->capacity - wc; j++)
                buf->buffer[j] = buf->buffer[j + wc];
            buf->size -=wc;
        }
        ++buf->size;
        buf->buffer[buf->size - 1] = src[i];
    }
    return len;
}
