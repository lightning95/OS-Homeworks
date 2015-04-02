#include <stdio.h>
#include <helpers.h>

const size_t BUF_CAP = 4096;

int main(int argc, char* argv[])
{
	char* args[argc];
	for (int i = 0; i + 1 < argc; ++i) 
		args[i] = argv[i + 1];
	ssize_t bc = 0, rc = 0;
	char buf[BUF_CAP];
	while (1)
	{
		ssize_t rc = read_until(STDIN_FILENO, buf + bc, BUF_CAP - bc, '\n');
		if (rc < 0)
			return 1;
		ssize_t last = 0;
		for (ssize_t i = bc; i < rc + bc; ++i)
			if (buf[i] == '\n')
			{
				buf[i] = 0;
				args[argc - 1] = buf + last;
				int status = spawn(args[0], args);
				buf[i] = '\n';
				if (status == 0 && write_(STDOUT_FILENO, buf + last, i - last + 1) < 0) 
					return 1;
				last = i + 1;
			}
		if (rc <= 0) 
			break;
		for (int i = last; i < rc + bc; ++i) 
			buf[i - last] = buf[i];
		bc = rc - last + bc;
	}
	args[argc - 1] = buf;
	buf[bc] = 0;
	int status = spawn(args[0], args);
	if ((status == 0 && write_(STDOUT_FILENO, buf, bc) < 0) || status == 1) 
		return 1;
	return rc == 0 ? 0 : 1;
}
