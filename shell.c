// ----------------------------------------------
// These are the only libraries that can be 
// used. Under no circumstances can additional 
// libraries be included
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "shell.h"

int find_location_of_pipe(command_t* p_cmd);   // Finds the location of the Pipe

// --------------------------------------------
// Currently only two builtin commands for this
// assignment exist, however in future, more could 
// be added to the builtin command array.
// --------------------------------------------
const char* valid_builtin_commands[] = {"cd", "exit", NULL};

// --------------------------------------------
// Implementation for each prototype below
// --------------------------------------------

// get the next token starting from line.  Return length of token.
int get_next_token(char* line, char** token)
{
  int i;                 // for loop control variable
  int start;             // start of some non-white space block
  int inwhite = TRUE;    // TRUE if "in" white space

  // go through the string one character at a time
  for (i = 0; line[i] != '\0'; i++) {  // NOTE:  i incremented ONLY here!
    if (iswhite(line[i])) {  //   did we find a white space?
      if (!inwhite) {        //   if not already in white space
	break;               //       then we got a new token
      }
      inwhite = TRUE;        //   remember the state that we are in white space
    }
    else {                   // not a white space character (ie. an argument)
      if (inwhite) {         // remember where we transitioned from a white
	start = i;           //   to non-white region
      }
      inwhite = FALSE;       // we are now not in a white space state
    }
  }

  if (!inwhite) {            // if we didn't end with a white space
    *token = line + start;   //     then we got a new token
    return i - start;
  }

  *token = NULL;             // if we get here, no tokens available
  return 0;
}

int count_tokens(char* line)
{
  int num_tokens = 0;
  char *token;
  int length = get_next_token(line, &token);
  
  while (length > 0) {
    num_tokens++;
    line = token + length;
    length = get_next_token(line, &token);
  }
  
  return num_tokens;
}

void parse(char* line, command_t* p_cmd)
{
  int i;
  int length;
  char *token;
  
  // get number of command line arguments
  p_cmd->argc = count_tokens(line);

  if (p_cmd->argc > 0) {
    // allocate space for arguments array (+ 1 for NULL)
    p_cmd->argv = (char **) malloc((p_cmd->argc+1) * sizeof(char *));

    for (i = 0; i < p_cmd->argc; i++) {   // for each argument...
      length = get_next_token(line, &token);
      line = token + length;
      p_cmd->argv[i] = (char *) malloc(length + 1);
      my_strncpy(p_cmd->argv[i], token, length);
    }
    p_cmd->argv[i] = NULL;
  }
  else {   // blank line entered
    p_cmd->argv = (char **) malloc(2 * sizeof(char *));
    p_cmd->argv[0] = (char *) malloc(1);
    p_cmd->argv[0][0] = '\0';
    p_cmd->argv[1] = NULL;
  }
  // allocate space and copy argv[0] into name
  p_cmd->name = (char *) malloc(my_strlen(p_cmd->argv[0])+1);
  my_strncpy(p_cmd->name, p_cmd->argv[0], my_strlen(p_cmd->argv[0]));
}

int execute(command_t* p_cmd)
{
  int i;
  int pid;
  int status;
  char* fullpath = find_fullpath(p_cmd);  // try to get the fullpath of command
  int pipe_position; // position of the pipe symbol
  int fd[2]; // used to store two ends of the pipe
  int child_pid1, child_pid2;

   

  // Determine weather or not we have a pipe in p_cmd->argv, if it is not equal to -1 then we hav  pipe else execute should run normally.
  if ((pipe_position = find_location_of_pipe(p_cmd)) != -1) {
    //printf("After find_location_of_pipe\n");
    //printf("%i\n", pipe_position);
    //printf("%s\n", fullpath);
    //printf("%s\n", p_cmd->argv[pipe_position]);

    
    
    pipe(fd);
    printf("entered pipe\n");


    if ((child_pid1 = fork()) == 0) { 
      //printf("entered pipe if cpid_1\n"); 
      p_cmd-> argv[pipe_position] = NULL; // this sets the childs pipe position = to null
      //printf("after if in childs_1 pipe position set to null\n");
      fullpath = find_fullpath(p_cmd);
      printf("fullpath = %s\n", fullpath);
      close(1); // closes the std out
      dup(fd[1]);  // Set stdout to be fd[1]
      close(fd[0]);
      execv(fullpath, p_cmd->argv);  // the execv command allows to run the process at fullpath with the given arguments

      printf("fullpath = %s\n", fullpath);
      i = 0;
      while (p_cmd->argv[i] != NULL) {
         printf("%s ", p_cmd->argv[i]);
	 i++;
      }
      printf("\n");
      
  }
    if ((child_pid2 = fork()) == 0) {
      printf("entered pipe if cpid_2\n");
      p_cmd->argv += (pipe_position + 1);
      p_cmd->argc -= (pipe_position + 1);
      p_cmd->name = p_cmd->argv[0];
      fullpath = find_fullpath(p_cmd);
      close(0); // closes the std out
      dup(fd[0]);  // Set stdout to be fd[1]
      close(fd[1]);
      execute(p_cmd);
      exit(1);
  }
  close(fd[0]);
  close(fd[1]);
  waitpid(child_pid1,NULL, 0);
  waitpid(child_pid2,NULL, 0); 
  printf("closed in the pipe if\n");

 return 0;
}
	

  if (fullpath == NULL) {
    fprintf(stderr, "fullpath is NULL in execute\n");
    return 0;
  }

  if ((pid = fork()) == 0) {	
    // child executes here
    // NOTE:  no need to free(fullpath) ---- ask yourself why?
    execv(fullpath, p_cmd->argv);
    perror("Execute terminated with an error condition!\n");
    exit(1);
  }

  return wait(&status);   // wait for child to exit()
}

char* find_fullpath(command_t* p_cmd)
{
  const int cmdname_len = my_strlen(p_cmd->name);  // length of cmd->name
  int i;                  // loop control variable
  int dir_start = 0;      // start position in pathvar of a directory
  struct stat sb;         // for stat() call
  char *pathvar = getenv("PATH");  // entire PATH string
  int pathvarlen = my_strlen(pathvar);  // length of PATH string
  char* fullpath;         // fullpath extracted from dir in PATH and cmd->name
  int fullpath_len;       // length of fullpath

  

  for (i = 0; i < pathvarlen+1; i++) {   // upto and including '\0' char
    if (pathvar[i] == ':') {             // found another directory in PATH
      // setup return of fullpath: pathlen+cmdlen+slash+nullchar
      fullpath_len = i-dir_start+cmdname_len+2;
      fullpath = malloc(fullpath_len);
      my_strncpy(fullpath, pathvar+dir_start, i-dir_start);
      dir_start = i+1;
      // build the fullpath string
      fullpath[i-dir_start] = '\0';    // first properly terminate directory
      my_strncat(fullpath, "/", fullpath_len);  // append a slash
      my_strncat(fullpath, p_cmd->name, fullpath_len);  // append the cmd name

      if (stat(fullpath, &sb) != -1 && S_ISREG(sb.st_mode)) {
	return fullpath;
      }
      free(fullpath);   // else not found...free up fullpath space and continue
    }
  }

  return NULL;
}

int is_builtin(command_t* p_cmd)
{
  int i;

  for (i = 0; valid_builtin_commands[i] != NULL; i++) {
    if (my_strequal(p_cmd->name, valid_builtin_commands[i]))
      return TRUE;
  }
  return FALSE;
}

void do_builtin(command_t* p_cmd)
{
  if (my_strequal(p_cmd->name, valid_builtin_commands[0]))
    chdir(p_cmd->argv[1]);

  // can add more commands ...
}

void cleanup(command_t* p_cmd)
{
  int i;
  
  free(p_cmd->name);
  p_cmd->name = NULL;
  for (i = 0; i < p_cmd->argc; i++) {
    free(p_cmd->argv[i]);
    p_cmd->argv[i] = NULL;
  }
  free(p_cmd->argv);
  p_cmd->argv = NULL;
  p_cmd->argc = 0;
}	

// String utility functions --- useful for parsing strings

int my_strlen(const char* s)
{
  int count = 0;
  while (*s++ != '\0')
    count++;

  return count;
}

// assumes properly terminated strings
int my_strequal(const char *s, const char *t)
{
  while (*s != '\0' && *s == *t) {
    s++; t++;
  }

  return *s == '\0' && *t == '\0';
}

int my_strncpy(char *dest, const char *src, int n)   // copy dest to src
{
  while (*src != '\0' && n--) {
    *dest++ = *src++;
  }
  *dest = '\0';
  
  return 0;
}

int my_strncat(char *dest, const char *src, int n)   // append dest to src
{
  while (--n > 0 && *dest != '\0')
    dest++;    // move to the end of dest
  
  while (*src != '\0' && n > 0) {
    *dest++ = *src++;
    n--;
  }
  *dest = '\0';

  return 0;
}

int iswhite(char c)
{
  return c == ' ' || c == '\t';
}

int find_location_of_pipe(command_t* p_cmd) {   // Finds the location of the Pipe
int i;

for (i = 0; i < p_cmd -> argc; i++) {
  if(my_strequal(p_cmd -> argv[i], "|")) {  // used the given strequal cmd to check if we had "|"
    printf("entered if statement in find_location_of_pipe\n"); // Test
    return i;
   }
  } 
return -1; // if no pipe symbol found return -1
}





