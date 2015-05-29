#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

struct execargs_t
{
	char** argv;
};

struct execargs_t new_execargs_t(int argc, char** argv);

int exec(struct execargs_t* args);

int runpiped(struct execargs_t** programs, size_t n);

//----------------------

ssize_t read_(int fd, void *buf, size_t count);

ssize_t write_(int fd, const void *buf, size_t count);

ssize_t read_until(int fd, void * buf, size_t count, char delimiter);

int spawn(const char * file, char * const argv []);

#endif
