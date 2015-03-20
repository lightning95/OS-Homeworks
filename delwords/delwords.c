#include <stdio.h>
#include <string.h>
#include <helpers.h>

const ssize_t BUF_CAPACITY = 4096;
const ssize_t MAX_WORD = 1024;

int main(int argn, char** args) 
{
	char buf[BUF_CAPACITY];
	ssize_t bc = 0;
	ssize_t len = strlen(args[1]);
	while (1)
	{
		ssize_t rc = read_(STDIN_FILENO, buf + bc, BUF_CAPACITY - bc);
		if (rc == 0)
		{
			if (bc > 0 && write_(STDOUT_FILENO, buf, bc) == -1)
				return 2;
			break;
		}
		if (rc < 0)
			return -rc;
		bc += rc;
		ssize_t last = 0;
		ssize_t end = 0;
		for (ssize_t i = 0; i < bc; )
		{
			int ok = 0;
			for (ssize_t j = i; j < bc && j - i < len && buf[j] == args[1][j - i]; ++j)
				++ok;
			if (ok > 0 && (ok == len || i + ok == bc))
			{
				if (write_(STDOUT_FILENO, buf + last, i - last) == -1)
						return 2;
				if (ok == len)
				{
					i += len;
					last = i;
				} else {
					// print to i - 1, shift checkout	
					for (ssize_t j = 0; j < ok; ++j)
						buf[j] = args[1][j];
					bc = ok;
					end = 1;
					break;
				}
			} else {
				if (write_(STDOUT_FILENO, buf + last, i - last + 1) == -1)
					return 2;
				last = i + 1;
				++i;
			}
		}
		if (end != 1)
			bc = 0;
	}	

	return 0;
}