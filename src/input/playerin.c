#include <string.h>
#include "input.h"
#include "joystick.h"	/* js_dead */
#include "../options.h"

static int getdev(const char *str)
{
	if (str) {
		if (!strcmp(str, "js0"))
			return inputdevs_fd[1] > -1;
		if (!strcmp(str, "js1") && inputdevs_fd[2] > -1)
			return 2;
	}
	return 0;
}

static void setpl2js()
{
	const char *s = "js0";
	union val v;
	if (inputdevs_fd[1] < 0)
		s = "js1";
	strcpy(v.str, s);
	setoption("player2", "input", v, 1);
	inputdevs_player[s[2]-'0'+1] = 2;
}

void initplayerinput()
{
	int devs[2];
	if (!num_joyst) {
		inputdevs_player[0] = 0;
		return;
	}
	inputdevs_player[0] = 1;
	devs[0] = getdev(getopt_str("player1", "input"));
	devs[1] = getdev(getopt_str("player2", "input"));
	if (devs[0] != devs[1]) {
		inputdevs_player[devs[0]] = 1;
		inputdevs_player[devs[1]] = 2;
	} else if (devs[0]) {
		unsetoption("player1", "input");
		inputdevs_player[devs[0]] = 2;
	} else if (js_dead)
		inputdevs_player[0] = 0;
	else
		setpl2js();
}
