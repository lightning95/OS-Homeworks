#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <sys/types.h>

#include "helpers.h"
#include <string.h>
#include <signal.h>

struct execargs_t new_execargs_t(int argc, char** argv)
{
    struct execargs_t ret;
    ret.argv = (char**) malloc((argc + 1) * sizeof(char*));
    for (int i = 0; i < argc; i++) 
        ret.argv[i] = strdup(argv[i]); 
    ret.argv[argc] = NULL;
    return ret;
}

int exec(struct execargs_t* args) 
{
    if (spawn(args->argv[0], args->argv) == -1)
        return -1;
    return 0;
}
 
int childn;
int* childa;

void sig_handler(int sig) {
    for (int i = 0; i < childn; i++) 
        kill(childa[i], SIGKILL);
    childn = 0;
}

int runpiped(struct execargs_t** programs, size_t n) 
{
	if (n == 0)
		return 0;
	int pipefd[n - 1][2];
	int child[n];	 
	for (int i = 0; i + 1 < n; i++) 
        if (pipe2(pipefd[i], O_CLOEXEC) < 0)
            return -1;

    for (int i = 0; i < n; i++) 
    {
		if (!(child[i] = fork())) 
		{
			if (i + 1 < n)
				dup2(pipefd[i][1], STDOUT_FILENO);
            if (i > 0)
				dup2(pipefd[i - 1][0], STDIN_FILENO);
			_exit(execvp(programs[i]->argv[0], programs[i]->argv));	
		}
        if (child[i] == -1)
            return -1;
	}
	for (int i = 0; i + 1 < n; i++) 
	{
		close(pipefd[i][1]);
		close(pipefd[i][0]);
	}
    
    childn = n;
    childa = (int*) child;
    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &sig_handler;
   
    if (sigaction(SIGINT, &act, NULL) < 0) 
        return -1;

	for (int i = 0, status; i < n; i++) 
        waitpid(child[i], &status, 0);
    childn = 0;
    return 0;
}


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

ssize_t read_until(int fd, void* buf, size_t count, char delimiter)
{
    size_t offset = 0, f = 0;
    ssize_t rr;
    while (f == 0 && (rr = read(fd, buf + offset, count)) > 0)
    {
        for (size_t i = offset; i < offset + rr && f == 0; ++i)
            if (*((char*)buf + i)  == delimiter)
                f = 1;
        offset += rr;
    }
    return rr == -1 ? -1 : offset;
}

int spawn(const char * file, char * const argv [])
{
    int res = fork();
    if (res > 0) 
    {
        int status;
        wait(&status);
        return status;
    } else if (res == 0) 
        exit(execvp(file, argv));
    else 
        return -1;
}
