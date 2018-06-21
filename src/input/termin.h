extern char unreadchr;

void set_input_mode();
void restore_input_mode();

int readchr();
int waitinput_stdin(unsigned msec);
