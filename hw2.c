/*
----------------------------------------------
These are the only libraries that can be used. Under no circumstances
can additional libraries be included!
----------------------------------------------
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "shell.h"

/*
-----------------------------------
  Main function 
  Arguments:	argc = number of arguments suppled by user
                argv = array of argument values
-----------------------------------
*/

int main( int argc, char** argv ) {
  command_t cmd;
  char line[MAXSTRLEN];   // commands limited to MAXSTRLEN characters for now
  char *fullpath;
  int done = FALSE;

  while (!done) {
    printf(">> ");
    fgets(line, MAXSTRLEN, stdin);
    line[my_strlen(line)-1] = '\0'; // get rid of newline

    parse(line, &cmd);

    if (my_strequal(cmd.name, "exit")) {
      done = TRUE;
    }
    else if (is_builtin(&cmd)) {
      do_builtin(&cmd);
    }
    else if ((fullpath = find_fullpath(&cmd)) != NULL) {
      // NOTE:  when using pipes, fullpath will not be NULL and thus enter "if"
      execute(&cmd);    // NOTE: find_fullpath() is called again in execute()
      free(fullpath);   // don't forget to free up fullpath else memory leak !
    }
    else if (line[0] != '\0') {   // non-blank line entered
      printf("invalid command\n");
    }

    cleanup(&cmd);
  }

  return 0;
} // end main function
