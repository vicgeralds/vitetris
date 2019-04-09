extern char cfgfilename[80];

void setcfgfilename(const char *argv0);

void readoptions();
void readoptions_from(FILE *fp, const char **sect_names, int num_sect_names);

/* returns 1 if file was written, 0 otherwise */
int writeconfig();

/* prints error message to stdout on failure */
void writeconfig_message();
