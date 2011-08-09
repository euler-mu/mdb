/* TODO: 
   make -r the default switch, no need to tell it to run
   add the more exotic registers to print_registers
   try to combine check_switches and check_switch into one function
   add command history
   implement breakpoint functionality
   implement a disassemble command ala GDB
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

//walk is shamelessly borrowed from Jeffrey A. Turkstra (http://turkeyland.net/)
void walk(void *a, int bytes) {
  int i, j;
  char *addr = a;

  while(((long)addr) % 8) addr++;

  for(i=0; i < bytes/8; i++) {
    printf("%p: ", addr);

    for(j = 0; j < 8; j++) 
      printf("%02hhx ", addr[j]);

    for(j = 0; j < 8; j++)
      printf("%c", isprint(addr[j]) ? addr[j] : '?');

    addr -=8;
    printf("\n");
  }
  
  return;
}

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
  printf("rax: 0x%lx      %lu\n", (*regs).rax, (*regs).rax);
  printf("rbx: 0x%lx      %lu\n", (*regs).rbx, (*regs).rbx);
  printf("rcx: 0x%lx      %lu\n", (*regs).rcx, (*regs).rcx);
  printf("rdx: 0x%lx      %lu\n", (*regs).rdx, (*regs).rdx);
  printf("rsi: 0x%lx      %lu\n", (*regs).rsi, (*regs).rsi);
  printf("rbp: 0x%lx      %lu\n", (*regs).rbp, (*regs).rbp);
  printf("rip: 0x%lx      %lu\n", (*regs).rip, (*regs).rip);
  printf("rsp: 0x%lx      %lu\n", (*regs).rsp, (*regs).rsp);
  if(all) {
    //print the other registers
  }
}

bool check_switches(char *input, char *switch1, char *switch2) {
  if(input != NULL && switch1 != NULL && switch2 != NULL) {
    if(strlen(input) == (strlen(switch1) + 1) || strlen(input) == strlen(switch1) || strlen(input) == (strlen(switch2) + 1) || strlen(input) ==  strlen(switch2)) {   
      return (strncmp(input, switch1, strlen(switch1)) == 0 || strncmp(input, switch2, strlen(switch2)) == 0);
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
}

bool check_switch(char *input, char *s) {
 if(input != NULL && s != NULL) {
    if(strlen(input) == (strlen(s) + 1) || strlen(input) == strlen(s)) {   
      return (strncmp(input, s, strlen(s)) == 0);
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
}

//takes the substring of a given string from indices [l, u) 
char *substr(char *input, int l, int u) {
  if(l < strlen(input) && strlen(input) >= u && u > l) {
    char *ret = malloc(sizeof(char) * (u - l));
    for(int i = 0; i < u - l; i++) {
      ret[i] = input[l + i];
    }
    return ret;
  } else {
    return input;
  }
}

void io_handler(char *input, pid_t child, int status){
  long lptrace;
  printf("(mdb)");
  int input_size = 1024, input_len = 0;
  input_len = getline(&input, &input_size, stdin);
  
  if(check_switches(input, "step", "s")) {
    lptrace = ptrace(PTRACE_SINGLESTEP, child, NULL, NULL);
    wait(&status);
  }
  else if(check_switches(input, "forward", "f")) {
    lptrace = ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    wait(&status);
  }
  
  else if(check_switches(input, "registers", "rg")) {
    struct user_regs_struct registers;  
    lptrace = ptrace(PTRACE_GETREGS, child, NULL, &registers);
    
    print_registers(&registers, FALSE);
  }
  
  else if(check_switch(input, "pid")) {
    printf("pid: %d\n", child);
  }
  else if(strncmp(input, "print", 5) == 0) {
    /* char *arg = substr(input, 6, strlen(input ) - 1); //strip the first space and the terminal '\n'
    printf("%s\n", arg);
    printf("%ld\n",strtoul(arg, NULL, 16));
    void *ptr = (void *) strtoul(arg,NULL,16);
    */
  }
  else if(strncmp(input, "examine", 7) == 0 || strncmp(input, "ex", 2) == 0) {
    int start = 0;
    char *saddress = malloc(sizeof(char) * 18);
    
    saddress[0] = '0';
    saddress[1] = 'x';
    
    for(int i = 0; i < input_len; i++) {
      if(i + 1 < input_len) {
	if(input[i] == '0' && input[i + 1] == 'x') {
	  i = i + 2;		
	  for(int c = 0; c < 12; c++) {
	    saddress[2 + c] = input[i+c];
	  }
	  start = i + 13;
	  break;
	}
      }
    }
    if(start == 0) {
      puts("error parsing input.\nproper usage: (ex)amine address num-bytes");
    } else {
      unsigned long address = strtoul(saddress, NULL, 16);
      
      int num_bytes = atoi(substr(input, start, strlen(input) - 1));
      walk((void *) address, num_bytes);
    }
  }
  else if(strncmp(input, "cmdline", 7) == 0) {
    forkxecutel("/bin/cat", "cat", build_proc_path(child, "cmdline"));
  }
  
  else if(strncmp(input, "maps", 7) == 0) {
    forkxecutel("/bin/cat", "cat", build_proc_path(child, "maps"));
  }
  
  else if(strncmp(input, "limits", 7) == 0) {
    forkxecutel("/bin/cat", "cat", build_proc_path(child, "limits"));
  } 
  else if(strncmp(input, "help", 4) == 0) {
    puts("(s)tep - step one instruction");
    puts("(f)orward - execute until next syscall");
    puts("(ex)amine address bytes - dumps memory from address to address - bytes");
    puts("pid - prints process id");
    puts("maps - cats /proc/pid/maps");
    puts("limits - cats /proc/pid/limits");
  } else {
    puts("Command not recognized. Type help for more information.");
  }
}

int main(int argc, char **argv) {
  int status = 0, flag = 0;
  char *input = malloc(sizeof(char) * 1024);
  if(argc == 1) {
    puts("usage: mdb file [options]\nsee mdb --help or mdb -h for more details");
    return 0;
  } else {
    for(int i = 0; i < argc; i++) {
      if(!flag) {
	if(check_switches(argv[i], "--attach", "-a")) {
	  if(i + 1 < argc) {
	    pid_t pid = atoi(argv[i+1]);
	    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
	    while(1)
	      io_handler(input, pid, status);
	  }
	}
	if(check_switches(argv[i], "--help", "-h")) {
	  puts("options:\n --attach pid or -a pid\n--run file or -r file");
	  flag = 1;
	  return 1;
	}
	if(check_switches(argv[i], "--run", "-r")) {
	  if(i + 1 < argc) {
	    pid_t child = fork();

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
	    
	      while(1) 
		io_handler(input, child, status);
	    }
	  }
	}
      }
    }  
    
    //no options were selected
    if(flag == FALSE) {
      puts("usage: mdb file [options]\nsee mdb --help or mdb -h for more details");
      return 0;
    }
  }
  _exit (WEXITSTATUS(status));
}
