#ifndef _OPTION_H
#define _OPTION_H
/* currently building the argc/argv stuff in a global context */
#define ARGV_MAX  255
#define ARGV_TOKEN_MAX  255
extern int    _argc;
extern char  *_argv[ARGV_MAX];

/*
 * Demo usage
  char* cmd="quit";
  str2argv(cmd);
  if(_argc){
    if(!strcmp(_argv[0],"quit")){
    	//stuff to handle quit command
	}
  }
  argv_dispose();
*/
void str2argv(char *s);
void argv_dispose();
#endif 
