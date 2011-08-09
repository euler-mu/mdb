#include <getopt.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef BOOL
#define BOOL
typedef int bool;
#endif

#define FALSE 0
#define TRUE 1

#ifndef ARGS_T
#define ARGS_T
struct arguments_t {
  char *input_file;
  pid_t process; 
} arguments;
#endif

#ifndef SOPTS
#define SOPTS
static const char *sopts = "a:r:h";
#endif

#ifndef ACTIONLEN
#define ACTIONLEN
static const int action_len = 1024;
#endif

bool RUNNING;//= FALSE;
bool ATTACHED; //= FALSE;
pid_t child_pid;
