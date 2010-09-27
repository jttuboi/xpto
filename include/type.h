#ifndef _TYPE_H_
#define _TYPE_H_

#define BOOL char
#define TRUE 1
#define FALSE 0

typedef struct {
	pid_t pid;
	int descriptor;
	BOOL is_interactive;
	pid_t pgid;
} shell_conf;


#endif
