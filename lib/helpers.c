#include <unistd.h>
#include "helpers.h"

ssize_t read_(int fd, void* buf, size_t count) 
{
    size_t offset = 0;
    ssize_t rr;
    while ((rr = read(fd, buf + offset, count - offset)) > 0) 
        offset += rr;
    return rr == -1 ? -1 : offset;        
}

ssize_t write_(int fd, const void* buf, size_t count) 
{
    size_t offset = 0;
    ssize_t rr = 0;
    while (offset < count && rr >= 0) 
        offset += rr = write(fd, buf + offset, count - offset);
    return rr == -1 ? -1 : offset;
}