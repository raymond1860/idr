#ifndef CONFIG_H
#define CONFIG_H 1

#define MAX_ENTRY_LEN  64
#define DEFAULT_CONFIG_FILE "id2reader.cfg"

int
	get_config(const char* configfile,const char* name,char* value);

#endif

