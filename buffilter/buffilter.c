#include "../lib/bufio.h"
#include "../lib/helpers.h"
#include <stdio.h>

#define BUF_SIZE 4096
#define ARG_SIZE 256

int main(int argc, char* argv[])
{
    char* args[ARG_SIZE];
	for (int i = 0; i + 1 < argc; ++i) 
		args[i] = argv[i + 1];

	struct buf_t * in = buf_new(BUF_SIZE);
	struct buf_t * out = buf_new(BUF_SIZE);

	char dest[BUF_SIZE];
	while (1)
	{
		ssize_t rc = buf_getline(STDIN_FILENO, in, dest);
		if (rc == 0) {
            buf_flush(STDOUT_FILENO, out, buf_size(out));
            return 0;
        }
		if (rc < 0)
			return 1;
		
		args[argc - 1] = dest;
		if (dest[rc - 1] == '\n') {
            rc--;
        }
        dest[rc] = 0;
		int status = spawn(args[0], args);
		if (status == 0) {
            dest[rc] = '\n';
            if (buf_write(STDOUT_FILENO, out, dest, rc + 1) == -1) {
                return 1;
            }
        }
	}
	return 0;
}
