#include "mdb.h"

bool check_switches(char *input, char *switch1, char *switch2) {
  if(input != NULL && switch1 != NULL) {
    if(switch2 == NULL) {
      if(strlen(input) == (strlen(switch1 + 1)) || strlen(input) == strlen(switch1)) {   
	return (strncmp(input, switch1, strlen(switch1)) == 0 || strncmp(input, switch2, strlen(switch2)) == 0);
      } else {
	return FALSE;
      }
    } else {
      if(strlen(input) == (strlen(switch1 + 1) ) || strlen(input) == strlen(switch1) || strlen(input) == (strlen(switch2) + 1) || strlen(input) ==  strlen(switch2)) {   
	return (strncmp(input, switch1, strlen(switch1)) == 0 || strncmp(input, switch2, strlen(switch2)) == 0);
      } else {
	return FALSE;
      }
    }
  } else {
    return FALSE;
  }
}

int numtokens(char *str) {
  int num = 0;
  char *tokens = strtok(str, " ,.");

  while(tokens != NULL) {
    tokens = strtok(NULL, " ,.");
    num++;
  }

  return num;
}

char **parse_arguments(char *input) {
  int numargs = numtokens(input), i = 1;
  char **args = malloc(sizeof(char) * (numargs + 1));

  args[0] = malloc(sizeof(char) * strlen(arguments.input_file));
  strcpy(args[0], arguments.input_file);

  char *tokens = strtok(input, " ,.");
  while(tokens != NULL) {
    args[i] = malloc(sizeof(char) * strlen(tokens));
    strcpy(args[i++], tokens);
    tokens = strtok(NULL, " ,.");    
  }
  return args;
}

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
