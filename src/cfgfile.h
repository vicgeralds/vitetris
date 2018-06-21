extern char cfgfilename[80];

void setcfgfilename(const char *argv0);

void readoptions();

/* returns 1 if file was written, 0 otherwise */
int writeconfig();

/* prints error message to stdout on failure */
void writeconfig_message();
