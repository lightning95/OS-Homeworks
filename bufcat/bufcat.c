#include "../lib/bufio.h"

#define BUF_SIZE 1024

int main(int argc, char** argv)
{
    struct buf_t * buf = buf_new(BUF_SIZE);
    ssize_t res, res2 = 0;
    while ((res = buf_fill(STDIN_FILENO, buf, buf_capacity(buf))) > 0 && res2 >= 0)
        res2 = buf_flush(STDOUT_FILENO, buf, res);
    return res < 0 || res2 < 0 ? 1 : 0;
}
