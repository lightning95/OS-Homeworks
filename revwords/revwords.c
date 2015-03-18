#include <stdio.h>
#include <helpers.h>

const ssize_t BUF_SIZE = 4100; // ~4096 + eps

int main() 
{
	char buf[BUF_SIZE];
	while (1) 
	{
		ssize_t rc = read_until(STDIN_FILENO, buf, BUF_SIZE, ' ');
		if (rc <= 0)
			return -rc;
		size_t last = 0;
		for (size_t j = 0; j < rc; ++j)
			if (buf[j] == ' ')
			{
				for (size_t i = last; i < last + (j - last) / 2; ++i) 
				{
					char tmp = buf[i];
					buf[i] = buf[j - (i - last) - 1];
					buf[j - (i - last) - 1] = tmp;
				}	
				last = j + 1;
			}
		for (size_t i = last; i < last + (rc - last) / 2; ++i) 
		{
			char tmp = buf[i];
			buf[i] = buf[rc - (i - last) - 1];
			buf[rc - (i - last) - 1] = tmp;
		}	
		ssize_t wc = write_(STDOUT_FILENO, buf, rc);
		if (wc == -1)
			return 2;
	}
	return 0;
}
