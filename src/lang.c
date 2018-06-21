#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lang.h"

int lang = SPELL_GB;

static int islatin1(const char *s)
{
	while (*s) {
		if (*s == '.')
			break;
		s++;
	}
	if (*s != '.')
		return 1;
	s++;
	if (strncmp(s, "ISO", 3) && strncmp(s, "iso", 3))
		return 0;
	s += 3;
	if (*s == '-')
		s++;
	if (strncmp(s, "8859", 4))
		return 0;
	s += 4;
	if (*s == '-')
		s++;
	return *s == '1' && (!s[1] || s[1] == '5');
}

void getlang()
{
	const char *s = getenv("LANG");
	if (!s)
		return;
	if (!strncmp(s, "en_US", 5) || !strncmp(s, "en_us", 5))
		lang = SPELL_US;
	else
		lang = SPELL_GB;
	if (islatin1(s))
		lang |= LATIN1;
}

void spellword(char *s)
{
	int c;
	if (lang & SPELL_GB)
		return;
	c = tolower(s[0]);
	if (c=='a') {
		if (!strcmp(s, "acw"))
			s[0] = 'c';
		else if (!strncmp(s+1, "nticlockw", 9)) {
			memmove(s+7, s+4, strlen(s+3));
			memcpy(s, "counter", 7);
		} else if (!strcmp(s+1, "lternative")) {
			s[8] = 'e';
			s[9] = '\0';
		}
	} else if (c=='c' && !strncmp(s+1, "olour", 5)) {
		s[4] = s[5];
		s[5] = s[6];
		if (s[5])
			s[6] = '\0';
	}
}
