#include "mdb.h"
#include "mdb_utils.h"

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

void io_handler(char *action) {
  if(check_switches(action, "step", "s")) {
    if(RUNNING){
      ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
    } else{
      puts("no process running.\nto execute a process, type run arguments");
    }
  
  } else if(strncmp(action, "run", 3) == 0) {
    if(RUNNING) {
      puts("Process already running. would you like to restart?");
    } else {
      char **input_args = parse_arguments(substr(action, 4, strlen(action) - 1));
      
      child_pid = fork();
      if(child_pid == 0) {
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);
	execv(arguments.input_file, input_args);
	if(errno == E2BIG) { printf("ERROR: argument list exceeds system limit.\n"); }
	else if(errno == EACCES) { printf("ERROR: the specified file has a locking or sharing violation.\n"); }
	else if(errno == ENOENT) {
	  printf("ERROR: file or directory not found!\n");
	  exit(-1);
	} else if(errno == ENOMEM) { printf("ERROR: not enough memory to execute process image.\n"); }
      } else if(child_pid < 0 ) {
	printf("ERROR: unable to fork\n");
      } else {
	//successful fork
	RUNNING = TRUE;
      }
    }
  
  } else if(check_switches(action, "registers", "rg")) {
    if(RUNNING) {
      struct user_regs_struct registers;  
      ptrace(PTRACE_GETREGS, child_pid, NULL, &registers);
      print_registers(&registers, FALSE);
    } else puts("no process running.\nto execute a process, type run arguments"); 
  
  } else if(strncmp(action, "examine", 7) == 0) {
    if(strlen(action) > 7) {
      
    } else {
      puts("usage: examine(up, down) address\nprints data in the range from [address + up, address - down]");
    }

  } else if(check_switches(action, "step", "s")) {
    if(RUNNING)
      ptrace(PTRACE_SINGLESTEP, child_pid, NULL, NULL);
    else puts("no process running.\nto execute a process, type run arguments");

  } else if(check_switches(action, "forward", "f")) {
    if(RUNNING)
      ptrace(PTRACE_SYSCALL, child_pid, NULL, NULL);
    else puts("no process running.\nto execute a process, type run arguments");
  
  } else if(strncmp(action, "pid", 3) == 0 && strlen(action) == 3) {
    if(RUNNING)
      printf("pid: %d\n", child_pid);
    else puts("no process running.\nto execute a process, type run arguments");
  
  } else if(strncmp(action, "help", 4) == 0) {
    puts("step, s            step one instruction");
    puts("forward, f         execute until next syscall");
    puts("registers, rg      print registers");
    puts("examine address bytes      dumps memory from address to address - bytes");
    puts("pid                prints process id");
  
  } else {
    puts("Command not recognized. Type help for more information.");
  }
}

void run(bool attach) {
  if(attach) { ptrace(PTRACE_ATTACH, arguments.process, NULL, NULL); RUNNING = TRUE; }
  char* action, shell_prompt[6];
  
  while(1) {
    snprintf(shell_prompt, sizeof(shell_prompt), "(mdb)");
    action = readline(shell_prompt);
    
    if(!action)
      break;

    add_history(action);
    io_handler(action);
  }
}

int main(int argc, char **argv) {
  int cmdopts = 0;

  RUNNING = FALSE;
  ATTACHED = FALSE;

  arguments.input_file = NULL;
  arguments.process = 0;
  
  cmdopts = getopt(argc, argv, sopts);
  while(cmdopts != -1) {
    switch(cmdopts) {
    case 'a':
      arguments.process = atoi(optarg);
      ATTACHED = TRUE;
      run(ATTACHED);
      break;
    case 'r':
      arguments.input_file = optarg;
      run(FALSE);
      break;
    case 'h':
      puts("\nBasic usage:");
      puts("  -a PID, --attach PID       attach mdb to process PID");
      puts("  -r FILE, --run FILE        debug FILE");
      break;
    default:
      break;
    }
    cmdopts = getopt(argc, argv, sopts);
  }
}
