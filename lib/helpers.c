#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <sys/types.h>

#include "helpers.h"
#include <string.h>
#include <signal.h>

int child_num;
int* childa;

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
 
void sig_handler(int sig) {
    for (int i = 0; i < child_num; i++) 
        kill(childa[i], SIGKILL);
    child_num = 0;
}

int check_handler()
{
    struct sigaction sigact;
    memset(&sigact, '\0', sizeof(sigact));
    sigact.sa_handler = &sig_handler;
    return sigaction(SIGINT, &sigact, NULL);
}

int runpiped(struct execargs_t** programs, size_t n) 
{
	if (n == 0)    return 0;
    int child[n], pipes[n - 1][2];
	for (int i = 0; i + 1 < n; i++) 
        if (pipe2(pipes[i], O_CLOEXEC) < 0)
            return -1;

    for (int i = 0; i < n; i++) 
		if (!(child[i] = fork())) 
		{
			if (i + 1 < n && dup2(pipes[i][1], STDOUT_FILENO) < 0)   
                return -1;
            if (i > 0 && dup2(pipes[i - 1][0], STDIN_FILENO) < 0)   
                return -1;
			_exit(execvp(programs[i]->argv[0], programs[i]->argv));	
		} else if (child[i] < 0)     
            return -1;

	for (int i = 0; i + 1 < n; i++) 
	{
		close(pipes[i][1]);
		close(pipes[i][0]);
	}
    
    child_num = n;
    childa = (int*) child;

    if (check_handler < 0)  
        return -1;

	for (int i = 0, status; i < n; i++) 
        waitpid(child[i], &status, 0);
    child_num = 0;
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
