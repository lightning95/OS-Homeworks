#include <stdio.h>
#include <helpers.h>

const ssize_t BUF_SIZE = 4096;

int main() 
{
	char buf[BUF_SIZE];
	while (1) 
	{
		ssize_t rc = read_until(STDIN_FILENO, buf, BUF_SIZE, ' ');
		if (rc <= 0)
			return -rc;
		int md = (buf[rc - 1] == ' ');
		for (size_t i = 0; i < (rc - md) / 2; i++) 
		{
			char tmp = buf[i];
			buf[i] = buf[rc - i - 1 - md];
			buf[rc - i - 1 - md] = tmp;
		}
		ssize_t wc = write_(STDOUT_FILENO, buf, rc);
		if (wc == -1)
			return 2;
	}
	return 0;
}
