/*
  TODO: !!stop using scanf!!
        make -r the default switch, no need to tell it to run


  /proc/<pid>/cmdline - process command line arguments
  /proc/<pid>/cwd - symlink to current working directory of the process
  /proc/<pid>/environ - process environment
  /proc/<pid>/exe - symlink to executable
  /proc/<pid>/fd/* - open file descriptors
  /proc/<pid>/maps - map of process memory
  /proc/<pid>/mem - entire process memory
*/

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

#define TRUE 1
#define FALSE 0
typedef int bool;

void forkxecutel(char *path, char *file, char *arguments) {
  pid_t pid = fork();
  if(pid == 0) {
    execl(path, file, arguments);
    if(errno == E2BIG)  {printf("ERROR: argument list exceeds system limit.\n");}
    if(errno == EACCES) {printf("ERROR: the specified file has a locking or sharing violation.\n");}
    if(errno == ENOENT) {
      printf("ERROR: file or directory not found!\n");
      exit(-1);
    }
    if(errno == ENOMEM) {printf("ERROR: not enough memory to execute process image.\n");}
  } else if(pid < 0 ) {
    printf("ERROR: unable to fork\n");
  } else {
    wait(&pid);
  }
}

//ilen counts the number of digits in an integer
int ilen(int n) {
  int count = 1;

  while((n / 10) != 0) {
    count++;
    n = n / 10;
  }

  return count;
}

//itoa turns an integer into a string representing the same integer
char *itoa(int n)
{
  char *ret;
  int len = ilen(n);
  ret = malloc(sizeof(char) * len);
  
  for(int i = 0; i < len; i++) {
    ret[len - i - 1] = (char) (((int) '0') + (n % 10));
    n = n / 10;
  }
  
  return ret;
}
 
char *build_proc_path(pid_t pid, char *file) {
  char *path = malloc(sizeof(char) * (ilen(pid) + 14));
  strncpy(path, "/proc/", 6);
  strcat(path, itoa(pid));
  strcat(path, "/");
  strcat(path, file);
  return path;
}

void print_registers(struct user_regs_struct *regs, bool all) {
#if __WORDSIZE == 64
  printf("rax: 0x%lx      %lu\n", (*regs).rax, (*regs).rax);
  printf("rbx: 0x%lx      %lu\n", (*regs).rbx, (*regs).rbx);
  printf("rcx: 0x%lx      %lu\n", (*regs).rcx, (*regs).rcx);
  printf("rdx: 0x%lx      %lu\n", (*regs).rdx, (*regs).rdx);
  printf("rsi: 0x%lx      %lu\n", (*regs).rsi, (*regs).rsi);
  printf("rbp: 0x%lx      %lu\n", (*regs).rbp, (*regs).rbp);
  printf("rip: 0x%lx      %lu\n", (*regs).rip, (*regs).rip);
  printf("rsp: 0x%lx      %lu\n", (*regs).rsp, (*regs).rsp);
  if(all) {
    //print mms and shit here
  }
#elif __WORDSIZE == 32
  printf("eax: 0x%l\n", (*regs).eax);
  printf("ebx: 0x%l\n", (*regs).ebx);
  printf("ecx: 0x%l\n", (*regs).ecx);
  printf("edx: 0x%l\n", (*regs).edx);
  printf("esi: 0x%l\n", (*regs).esi);
  printf("ebp: 0x%l\n", (*regs).ebp);
  printf("eip: 0x%l\n", (*regs).eip);
  printf("esp: 0x%l\n", (*regs).esp);
  if(all) {
    //print mms and shit here
  }
#endif
}

bool check_switches(char *input, char *switch1, char *switch2) {
  if(input != NULL && switch1 != NULL) {
    if(strlen(input) == strlen(switch1) || strlen(input) == strlen(switch2))
      return (strcmp(input, switch1) == 0 || strcmp(input,switch2) == 0);
    else
      return FALSE;
  }
}

int main(int argc, char **argv) {
  int status = -99, flag = 0;
  char *input = malloc(sizeof(char) * 1024);
  if(argc == 1) {
    printf("usage: mdb file [options]\nsee mdb --help or mdb -h for more details\n");
    return 0;
  } else {
    for(int i = 0; i < argc; i++) {
      if(!flag) {
	if(check_switches(argv[i], "--attach", "-a")) {
	  if(i + 1 < argc) {
	    pid_t pid = atoi(argv[i+1]);
	    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
	    //do stuff
	  }
	}
	if(check_switches(argv[i], "--help", "-h")) {
	  printf("options:\n --attach pid or -a pid\n--run file or -r file\n");
	  flag = 1;
	  return 1;
	}
	if(check_switches(argv[i], "--run", "-r")) {
	  if(i + 1 < argc) {
	    pid_t child = fork();
	    long lptrace;
	    
	    if(child == 0) {
	      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	      int start = 0;
	      
	      for(int j = 0; j < strlen(argv[i+1]); j++) {
		if(argv[i + 1][j] == ' ') break;
		if(argv[i + 1][j] == '/') {
		  
		  if(j + 1 < strlen(argv[i+1])){
		    start = j + 1; //start 1 char past the slash
		  }
		}
	      }
	      char *file = malloc(sizeof(char)*(strlen(argv[i+1]) - start));
	      for(int j = 0; j < strlen(argv[i+1]) - start; j++) {
		file[j] = argv[i + 1][start + j];
	      }

	      execl(argv[i + 1], file, NULL);
	    } else {
	      wait(&status);
	      if(WIFEXITED(status)) _exit (WEXITSTATUS(status));
	    
	      while(1) {
		if(WIFEXITED(status)) break;
		
		printf("(mdb)");
		scanf("%s", input);
		if(check_switches(input, "step", "s")) {
		  lptrace = ptrace(PTRACE_SINGLESTEP, child, NULL, NULL);
		  wait(&status);
		}
		if(check_switches(input, "forward", "f")) {
		  lptrace = ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		  wait(&status);
		}

		if(check_switches(input, "registers", "rg")) {
		  struct user_regs_struct registers;  
		  lptrace = ptrace(PTRACE_GETREGS, child, NULL, &registers);
		  
		  print_registers(&registers, FALSE);
		}
		
		if(strncmp(input, "pid", 3) == 0) {
		  printf("pid: %d\n", child);
		}

		if(strncmp(input, "print", 5) == 0) {
		  printf("%s\n", input);
		}

		if(strncmp(input, "cmdline", 7) == 0) {
		  forkxecutel("/bin/cat", "cat", build_proc_path(child, "cmdline"));
       		}

		if(strncmp(input, "maps", 7) == 0) {
		  forkxecutel("/bin/cat", "cat", build_proc_path(child, "maps"));
       		}

		if(strncmp(input, "limits", 7) == 0) {
		  forkxecutel("/bin/cat", "cat", build_proc_path(child, "limits"));
		}
		//usleep(1000);
	      }
	    }
	  }
	}
      }
    }

    //no options were selected
    if(flag == FALSE) {
      printf("usage: mdb file [options]\nsee mdb --help or mdb -h for more details\n");
      return 0;
    }
  }
  _exit (WEXITSTATUS(status));
}
