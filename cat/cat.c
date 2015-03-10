#include <stdio.h>
#include <helpers.h>

const ssize_t BUF_SIZE = 1024;

int main() 
{
	char buf[BUF_SIZE];
	while (1) 
	{
		ssize_t count = read_(STDIN_FILENO, buf, BUF_SIZE);
		if (count == -1 || count == 0)
			return count;
		ssize_t write_rs = write_(STDOUT_FILENO, buf, count);
		if (write_rs == -1)
			return 2;
	}
	return 0;
}
