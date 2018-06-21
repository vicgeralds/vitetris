/* Functions to initialize player input. */

/* returns 1 for "js0" if active,
	   2 for "js1" if active
	   0 otherwise	*/
int getinputdev(const char *str);

void initplayerinput(int *devs);
