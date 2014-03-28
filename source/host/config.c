#include "platform.h"
#include "config.h"

//#define MAXLEN 80






/*
 * trim: get rid of trailing and leading whitespace...
 *       ...including the annoying "\n" from fgets()
 */
char *
trim (char * s)
{
  /* Initialize start, end pointers */
  char *s1 = s, *s2 = &s[strlen (s) - 1];

  /* Trim and delimit right side */
  while ( (isspace (*s2)) && (s2 >= s1) )
    s2--;
  *(s2+1) = '\0';

  /* Trim left side */
  while ( (isspace (*s1)) && (s1 < s2) )
    s1++;

  /* Copy finished string */
  strcpy (s, s1);
  return s;
}

/*
 * parse external parameters file
 *
 * NOTES:
 * - There are millions of ways to do this, depending on your
 *   specific needs.
 *
 * - In general:
 *   a) The client will know which parameters it's expecting
 *      (hence the "struct", with a specific set of parameters).
 *   b) The client should NOT know any specifics about the
 *      configuration file itself (for example, the client
 *      shouldn't know or care about it's name, its location,
 *      its format ... or whether or not the "configuration
 *      file" is even a file ... or a database ... or something
 *      else entirely).
 *   c) The client should initialize the parameters to reasonable
 *      defaults
 *   d) The client is responsible for validating whether the
 *      pararmeters are complete, or correct.
 */
int
	get_config(const char* configfile,const char* name,char* value)
{
  int found=0;
  char *s, buff[256];
  FILE *fp = fopen (configfile?configfile:DEFAULT_CONFIG_FILE, "r");
  if (fp == NULL)
  {
    return -ENOENT;
  }

  /* Read next line */
  while ((s = fgets (buff, sizeof buff, fp)) != NULL)
  {
  	/* Parse name/value pair from line */
    char lhs[MAX_ENTRY_LEN], rhs[MAX_ENTRY_LEN];
    /* Skip blank lines and comments */
    if (buff[0] == '\n' || buff[0] == '#')
      continue;    
    s = strtok (buff, "=");
    if (s==NULL)
      continue;
    else
      strncpy (lhs, s, MAX_ENTRY_LEN);
    s = strtok (NULL, "=");
    if (s==NULL)
      continue;
    else
      strncpy (rhs, s, MAX_ENTRY_LEN);
    trim (rhs);

    /* Copy into correct entry in parameters struct */
    if (strcmp(name,lhs)==0){
    	strncpy (value,rhs, MAX_ENTRY_LEN);
    	found++;
    	break;
    }      
  }

  /* Close file */
  fclose (fp);
  return found?0:-ENOENT;
}
