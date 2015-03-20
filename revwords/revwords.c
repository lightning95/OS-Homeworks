#include <stdio.h>
#include <helpers.h>

const ssize_t BUF_CAPACITY = 4100; // ~4096 + eps
const ssize_t MAX_WORD = 4096;

void reverse(ssize_t len, char* buf)
{
	for (size_t i = 0; i < len / 2; ++i) 
	{
		char tmp = buf[i];
		buf[i] = buf[len - i - 1];
		buf[len - i - 1] = tmp;
	}	
}

int main() 
{
	char buf[BUF_CAPACITY];
	ssize_t bc = 0;
	while (1) 
	{
		ssize_t rc = read_until(STDIN_FILENO, buf + bc, BUF_CAPACITY - bc, ' ');
		if (rc < 0)
			return -rc;
		if (rc == 0)
		{
			if (bc == 0)
				break;
			reverse(bc, buf);
			if (write_(STDOUT_FILENO, buf, bc) == -1)
				return 2;
			break;
		}
		ssize_t i = bc, last;
		bc += rc;		
		for (last = 0; i < bc; ++i)
			if (buf[i] == ' ')
			{
				reverse(i - last, buf + last);
				if (write_(STDOUT_FILENO, buf + last, i - last + 1) == -1)
					return 2;
				last = i + 1;
			}
		if (bc - last == MAX_WORD) // clean buf
		{
			reverse(bc, buf);
			if (write_(STDOUT_FILENO, buf, bc) == -1)
				return 2;
			last = bc;
		}
		bc -= last;
		for (i = 0; i < bc; ++i) // shift buf
			buf[i] = buf[i + last];
	}
	return 0;
}
