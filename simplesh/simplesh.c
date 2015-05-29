#define _GNU_SOURCE         
#include "helpers.h"
#include "bufio.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

const int BUF_SIZE = 4096;
const int ARGS_NUM = 2048;

void sigint_handler(int sig) {}

int is_splitter(char c)
{
	return c == '|' || c == '\0' || c == ' ' || c == '\t';
}

int find_arguments(char* s, int shift)
{
	if (s[shift] == '\0')
        return 0;
    int count = 0;
    for (int i = shift + 1; 1; i++) 
    {
        count += (!is_splitter(s[i - 1]) && is_splitter(s[i])) ? 1 : 0;
        if (s[i] == '\0' || s[i] == '|')
            return count;
    }
    return count;
}

char* get_arg(char*s, int *shift)
{
	int i = *shift;
	for (; s[i] == ' ' || s[i] == '\t'; i++) {} // skip ' ' && '\t'
	int start = i;
	for (; !is_splitter(s[i]); i++) {} // until the end
	*shift = i;
	return start == i ? 0 : strndup(s + start, i - start);
}

void move_to_next(char* s, int *shift)
{
	int i = *shift;
	for (; s[i] == ' ' || s[i] == '\t'; i++){} // skip ' '  && '\t'
	*shift = s[i] == '|' ? i + 1 : i; // move over '|'
}

int main()
{
	struct sigaction sigact;
    memset(&sigact, '\0', sizeof(sigact));
    sigact.sa_handler = &sigint_handler;
   
    if (sigaction(SIGINT, &sigact, NULL) < 0)
        return 1;

	struct buf_t* buf = buf_new(BUF_SIZE); 
	char str[BUF_SIZE];
    while(1)
    {
    	if (write_(STDOUT_FILENO, "$ ", 2) < 1)
    		return 1;
    	int rc = buf_getline(STDIN_FILENO, buf, str);
    	if (rc == 0)
    		return 0;
    	if (rc < 0)
    	{
    		if (write_(STDOUT_FILENO, "\n$ ", 2) < 1)
    			return 1;
    		continue;
    	}
    	str[rc] = '\0';
    	struct execargs_t* arguments[ARGS_NUM];
    	int k = 0;
    	for (int shift = 0; 1; k++)
    	{
    		int argc = find_arguments(str, shift);
            if (argc == 0)
                break;
            char* argv[argc]; 
            for (int i = 0; i < argc; i++) {
                argv[i] = get_arg(str, &shift);
            }

            arguments[k] = (struct execargs_t*) malloc(sizeof(struct execargs_t));
            *arguments[k] = new_execargs_t(argc, argv);  

            move_to_next(str, &shift);
        }
        runpiped(arguments, k);
    }

    return 0;
}
