#include <stdio.h>
#include <string.h>
#include "hiscore.h"
#include "cfgfile.h"
#include "lang.h"
#include "game/tetris.h"
#include "config.h"

struct hiscore hiscores[10] = {{"", 0}};

static const char last_chars[8] = "Z≈ƒ÷!?‹";

void writehiscores(FILE *fp);

static void addhiscore(struct hiscore *hs)
{
	int i;
	if (hs->score > hiscores[0].score)
		i = 0;
	else {
		i = 10;
		while (hs->score > hiscores[i-1].score)
			i--;
		if (i == 10 || hs->score == hiscores[i-1].score &&
			       hs->lines == hiscores[i-1].lines &&
			       !strcmp(hs->name, hiscores[i-1].name))
			return;
	}
	memmove(hiscores+i+1, hiscores+i, (9-i)*sizeof(struct hiscore));
	hiscores[i] = *hs;
}

/* valid characters [A-Z0-9 .!?-≈ƒ÷‹] */
static int encodehiscore_name(const char *name, FILE *fp, int backcomp)
{
	int doshack = 0;
	char s[8];
	int i;
	unsigned char c;
	char *p;

	for (i = 0; c = name[i]; i++) {
		if (c == ' ')
			c = 3;
		else if (p = strchr(last_chars, c))
			c = 44 + p - last_chars;
		else if (c <= '9')
			c -= 44 - (c >= '6');
		else
			c -= 50;
		s[i] = c;
	}
	s[i] = 60;
	if (i < 7)
		s[i+1] = '\0';
	i = 0;
	while (i < 8 && s[i]) {
		switch (i % 4) {
		case 0:
			c = s[i] | (s[i+1] & 3)<<6;
			if (c =='\r' && !backcomp) {
				c = 255;
				doshack = 1;
			}
			break;
		case 1:
			c = s[i]>>2 | (s[i+1] & 15)<<4;
			break;
		case 2:
			c = s[i]>>4 | s[i+1]<<2;
			i++;
		}
		if (!c || c == '\n')
			c = 255;
		/* Ctrl-Z means end-of-file in MS text mode */
		else if (c == 0x1A && !backcomp) {
			c = 254;
			doshack = 1;
		}
		putc(c, fp);
		if (!s[i])
			break;
		i++;
	}
	return doshack;
}

static int decodehiscore_name(unsigned char *line, char *dest)
{
	int len = strlen(line);
	int i = 0;
	int j = 0;
	unsigned char c;

	while (i < 8) {
		if (j >= len)
			return 0;
		if (line[j] == 254)
			line[j] = 0x1A;  /* Ctrl-Z */
		switch (i % 4) {
		case 0:
			if (line[j] == 255)
				line[j] = '\r';
			c = line[j] & 63;
			break;
		case 1:
			c = line[j] >> 6;
			j++;
			if (line[j] == 255)
				line[j] = 0;
			c |= (line[j] & 15) << 2;
			break;
		case 2:
			c = line[j] >> 4;
			j++;
			if (line[j] == 255)
				line[j] = '\n';
			c |= (line[j] & 3) << 4;
			break;
		case 3:
			c = line[j] >> 2;
			j++;
		}
		if (c == 60)
			break;
		if (c == 0)
			return 0;
		if (c == 3)
			c = ' ';
		else if (c < 15)
			c += 44 - (c > 10);
		else if (c < 40)
			c += 50;
		else if (c >= 44 && c <= 50)
			c = last_chars[c-44];
		else
			return 0;
		dest[i] = c;
		i++;
	}
	dest[i] = '\0';
	if (i < 8 && i%4 < 3)
		j++;
	if (j > len)
		return 0;
	return j;
}

static int encodehiscore(struct hiscore *hs, FILE *fp, int backcomp)
{
	uint_least32_t d;
	int doshack = encodehiscore_name(hs->name, fp, backcomp);
	int c;
	d = hs->score;
	do {
		c = (d & 127)+20;
		if (c == 0x1A && !backcomp) {
			c = 19;
			doshack = 1;
		}
		putc(c, fp);
	} while (d >>= 7);
	putc((hs->score & 7)+2, fp);
	d = hs->startlevel+97+(hs->score & 7);
	putc(d, fp);
	putc(hs->level+d, fp);
	d = hs->lines;
	do putc((d & 63)+64, fp);
	while (d >>= 6);
	return doshack;
}

static int iseol(int c)
{
	return !c || c =='\n' || c=='\r';
}

static int decodehiscore(unsigned char *line, struct hiscore *hs)
{
	int i = decodehiscore_name(line, hs->name);
	int c;
	if (!i || iseol(line[i]))
		return 0;
	line += i;
	if (line[0] == 19)
		line[0] = 0x1A;
	c = (line[0]-20 & 7)+2;
	for (i = 1; line[i] != c; i++)
		if (iseol(line[i]))
			return 0;
	c = i;
	hs->score = 0;
	while (i) {
		i--;
		hs->score <<= 7;
		if (line[i] < 19)
			return 0;
		hs->score |= line[i]-20;
	}
	i = c+1;
	if (iseol(line[i]) || iseol(line[i+1]))
		return 0;
	hs->level = line[i+1]-line[i];
	c = line[i]-97-(hs->score & 7);
	if (c < 0 || c > 10 || hs->level < c)
		return 0;
	hs->startlevel = c;
	line += i+2;
	i = 0;
	while (!iseol(line[i]))
		i++;
	if (!i)
		return 0;
	hs->lines = 0;
	while (i) {
		i--;
		hs->lines <<= 6;
		if (line[i] < 64)
			return 0;
		hs->lines |= line[i]-64;
	}
	return 1;
}

static int readhiscores_fp(FILE *fp, char *line, int try)
{
	struct hiscore hs;
	while (fgets(line, 16, fp)) {
		if (decodehiscore(line, &hs))
			addhiscore(&hs);
		else if (try)
			return 0;
		try = 0;
	}
	fclose(fp);
	return 1;
}

static void readhiscores_config(FILE *fp, char *line)
{
	while (fgets(line, 16, fp))
		if (line[0] == '[' && !strncmp(line+1, "hiscore", 7))
			break;
	if (!feof(fp))
		readhiscores_fp(fp, line, 0);
	else
		fclose(fp);
}

static void mergehiscores(const char *filename, char *line)
{
	FILE *fp;
	fp = fopen(filename, "r");
	if (fp && !readhiscores_fp(fp, line, 1))
		readhiscores_config(fp, line);
}

#ifdef UNIX
static int savehiscores_global()
{
	FILE *fp = fopen(HISCORE_FILENAME, "w");
	if (!fp)
		return 0;
	writehiscores(fp);
	fclose(fp);
	return 1;
}
#else
#define savehiscores_global() 0
#endif

int readhiscores(const char *filename)
{
	FILE *fp;
	char line[16];
	if (filename)
		mergehiscores(filename, line);
	if (fp = fopen(cfgfilename, "r"))
		readhiscores_config(fp, line);
#ifdef UNIX
	if (fp = fopen(HISCORE_FILENAME, "r"))
		readhiscores_fp(fp, line, 0);
	if (filename)
		savehiscores_global();
#endif
	return (hiscores[0].score != 0);
}

void writehiscores(FILE *fp)
{
	char doshack[10] = {0};
	int i = 0;
	while (i < 10 && hiscores[i].score) {
		doshack[i] = encodehiscore(&hiscores[i], fp, 0);
		putc('\n', fp);
		i++;
	}
#ifdef UNIX
	for (i=0; i < 10; i++)
		if (doshack[i]) {
			encodehiscore(&hiscores[i], fp, 1);
			putc('\n', fp);
		}
#endif
}

int savehiscore(const char *name)
{
	struct hiscore hs;
	int n = strlen(name);
	if (!n)
		return 1;
	while (name[n-1] ==' ')
		n--;
	if (!n)
		return 1;
	strcpy(hs.name, name);
	hs.name[n] = '\0';
	hs.score = player1.score;
	hs.startlevel = player1.startlevel;
	hs.level = player1.level;
	hs.lines = player1.lines;
	addhiscore(&hs);
	n = writeconfig();
	return (savehiscores_global() || n);
}

const char *gethiscorename(int i, char *buf)
{
	const char lat1[5] = "≈ƒ÷‹";
	const char ascii[5] = "AAOU";
	const char *nm = hiscores[i].name;
	const char *p;
	if (lang & LATIN1)
		return nm;
	i = 0;
	while (*nm) {
		if (p = strchr(lat1, *nm))
			buf[i] = ascii[p-lat1];
		else
			buf[i] = *nm;
		nm++;
		i++;
	}
	buf[i] = '\0';
	return buf;
}

int gethiscorelist(char *s)
{
	char name[8];
	struct hiscore *hs;
	int i = 0;
	int n;
	while (i < 10 && hiscores[i].score) {
		hs = hiscores+i;
		n = sprintf(s, "%2d. %s", i+1, gethiscorename(i, name));
		memset(s+n, ' ', 12-n);
		s += 12;
		n = sprintf(s, "%7ld  %d-%d", (long) hs->score,
					      hs->startlevel, hs->level);
		memset(s+n, ' ', 15-n);
		s += 15;
		sprintf(s, "%3d", hs->lines);
		strcpy(s+3, i<9 ? " \n" : "\n");
		s += 5;
		i++;
	}
	return i;
}
