#include <stdio.h>
#include <string.h>
#include "hiscore.h"
#include "cfgfile.h"
#include "options.h"
#include "lang.h"
#include "game/tetris.h"
#include "config.h"

#define COLS_ATYPE "    Name      Score  Lvl  Lines"
#define COLS_BTYPE "    Name      Score  Lvl  Ht"
#define COLS_40L   "    Name      Time   TPM"

int hiscores_read = 0;
int num_hiscores = 0;
const char *hiscore_columns = COLS_ATYPE;

struct hiscore {
	char	      name[8];
	int_least32_t score;
	unsigned char level;
	unsigned char startlvl_ht;	/* startlevel or height */
	short	      lines_tpm;	/* lines or TPM */
};

static struct hiscore hiscores_atype[10];
static struct hiscore hiscores_btype[10];
static struct hiscore hiscores_40L[10];

static struct hiscore *hiscores = hiscores_atype;

static const char last_chars[8] = "Z≈ƒ÷!?‹";

void writehiscores(FILE *fp, const char *sect_name);

int ishiscore()
{
	if (hiscores == hiscores_atype) {
		if (player1.score < 12000)
			return 0;
	} else if (player1.lines > 0 ||
		   game.mode == MODE_1P_B   && player1.lineslimit != 25 ||
		   game.mode == MODE_1P_40L && (player1.lineslimit != 40 ||
			   			player1.score >= 60000))
		return 0;
	if (!num_hiscores)
		return 1;
	return isbetterscore(9);
}

static int is_better_score(int_least32_t score, int i)
{
	if (!hiscores[i].score)
		return 1;
	if (hiscores == hiscores_40L)
		return score < hiscores[i].score;
	return score > hiscores[i].score;
}

int isbetterscore(int i)
{
	return is_better_score(player1.score, i);
}

static void addhiscore(struct hiscore *hs)
{
	int n = 10;
	int i;
	if (is_better_score(hs->score, 0))
		i = 0;
	else {
		i = n;
		while (is_better_score(hs->score, i-1))
			i--;
		if (i == n || hs->score == hiscores[i-1].score &&
			      hs->lines_tpm == hiscores[i-1].lines_tpm &&
			      !strcmp(hs->name, hiscores[i-1].name))
			return;
	}
	memmove(hiscores+i+1, hiscores+i, (n-1-i)*sizeof(struct hiscore));
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

/* at end of line */

static void encode_lines_tpm(struct hiscore *hs, FILE *fp)
{
	int d = hs->lines_tpm;
	do putc((d & 63)+64, fp);
	while (d >>= 6);
	putc('\n', fp);
}

static int iseol(int c)
{
	return !c || c =='\n' || c=='\r';
}

static int decode_lines_tpm(unsigned char *s, struct hiscore *hs)
{
	int i = 0;
	while (!iseol(s[i]))
		i++;
	if (!i)
		return 0;
	hs->lines_tpm = 0;
	while (i) {
		i--;
		hs->lines_tpm <<= 6;
		if (s[i] < 64)
			return 0;
		hs->lines_tpm |= s[i]-64;
	}
	return 1;
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
	d = hs->startlvl_ht+97+(hs->score & 7);
	putc(d, fp);
	putc(hs->level+d, fp);
	encode_lines_tpm(hs, fp);
	return doshack;
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
	hs->startlvl_ht = c;
	line += i+2;
	return decode_lines_tpm(line, hs);
}

static void scoretokey(int_least32_t score, char *key)
{
	int i;
	for (i=0; score; i++) {
		key[i] = 'a'+ score % 26;
		score /= 26;
	}
	key[i] = '\0';
}

static int_least32_t keytoscore(const char *key)
{
	int_least32_t score = 0;
	int i;
	for (i=0; key[i]; i++) {
		if (key[i] >='0' && key[i] <='9' && !key[i+1])
			break;
		if (key[i] < 'a' || key[i] > 'z')
			return 0;
	}
	for (i--; i>=0; i--) {
		score *= 26;
		score += key[i]-'a';
	}
	return score;
}

static int checkbyte(struct hiscore *hs)
{
	return 'A'+ hs->score % 27 + hs->name[0] % 13 + hs->name[1] % 13;
}

static void encodehiscore_opt(struct hiscore *hs, FILE *fp)
{
	char key[8];
	scoretokey(hs->score, key);
	fputs(key, fp);
	putc('=', fp);
	putc('A'+ hs->level + hs->startlvl_ht*10, fp);
	encodehiscore_name(hs->name, fp, 0);
	putc(checkbyte(hs), fp);
	encode_lines_tpm(hs, fp);
}

static int decodehiscore_opt(struct option *o, struct hiscore *hs)
{
	char line[16];
	char *s = opt_longstr(o);
	int i;
	hs->score = keytoscore(opt_key(o));
	if (!hs->score || !s || s[0] < 'A' || s[0] > 'A'+59)
		return 0;
	i = s[0]-'A';
	hs->level       = i % 10;
	hs->startlvl_ht = i / 10;
	s++;
	if (strlen(s) > 15)
		return 0;
	strcpy(line, s);
	i = decodehiscore_name(line, hs->name);
	s += i;
	return i>0 && *s == checkbyte(hs) && decode_lines_tpm(s+1, hs);
}

static void gethiscores_opt(const char *sect_name)
{
	struct hiscore hs;
	struct option *o = getoptions(sect_name);
	for (; o; o=o->next) {
		if (decodehiscore_opt(o, &hs)) {
			addhiscore(&hs);
			unsetoption(sect_name, opt_key(o));
		}
	}
}

static int readhiscores_fp(FILE *fp, char *line, int try)
{
	struct hiscore hs;
	hiscores = hiscores_atype;
	while (fgets(line, 20, fp)) {
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
	const char *sect_names[2] = {"btype", "40l"};
	int wasempty = options_empty;
	int i;
	readoptions_from(fp, sect_names, 2);
	hiscores = hiscores_btype;
	gethiscores_opt("btype");
	hiscores = hiscores_40L;
	gethiscores_opt("40l");
	if (wasempty)
		freeoptions("");
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

#if UNIX && defined(HISCORE_FILENAME)
static int savehiscores_global()
{
	FILE *fp = fopen(HISCORE_FILENAME, "w");
	if (!fp)
		return 0;
	writehiscores(fp, NULL);
	fclose(fp);
	return 1;
}
#else
#define savehiscores_global() 0
#endif

void readhiscores(const char *filename)
{
	FILE *fp;
	char line[16];
	if (filename)
		mergehiscores(filename, line);
	if (fp = fopen(cfgfilename, "r"))
		readhiscores_config(fp, line);
#if UNIX && defined(HISCORE_FILENAME)
	if (fp = fopen(HISCORE_FILENAME, "r"))
		readhiscores_fp(fp, line, 0);
	if (filename)
		savehiscores_global();
#endif
	sethiscorecontext();
	hiscores_read = 1;
}

void sethiscorecontext()
{
	int mode;
	int lines;
	if (game.state == GAME_NULL)
		mode = getopt_int("", "mode");
	else
		mode = game.mode;
	hiscores = hiscores_atype;
	num_hiscores = 10;
	hiscore_columns = COLS_ATYPE;
	if (mode & MODE_LINECLEAR) {
		if (game.state != GAME_NULL && (mode == MODE_1P_B ||
						mode == MODE_1P_40L))
			lines = player1.lineslimit;
		else if (!(lines = getopt_int("", "lines")))
			lines = (mode & MODE_40L) ? 40 : 25;
		if (lines == 25) {
			hiscores = hiscores_btype;
			hiscore_columns = COLS_BTYPE;
		}
		if (lines == 40) {
			hiscores = hiscores_40L;
			hiscore_columns = COLS_40L;
		}
	}
	while (num_hiscores && !hiscores[num_hiscores-1].score)
		num_hiscores--;
}

void writehiscores(FILE *fp, const char *sect_name)
{
	char doshack[10] = {0};
	int i;
	if (sect_name) {
		if (!strcmp(sect_name, "btype"))
			hiscores = hiscores_btype;
		else if (!strcmp(sect_name, "40l"))
			hiscores = hiscores_40L;
		else
			return;
		for (i=0; i<10 && hiscores[i].score; i++)
			encodehiscore_opt(&hiscores[i], fp);
		return;
	}
	for (i=0; i<10 && hiscores_atype[i].score; i++)
		doshack[i] = encodehiscore(&hiscores_atype[i], fp, 0);
#ifdef UNIX
	for (i=0; i<10; i++)
		if (doshack[i])
			encodehiscore(&hiscores_atype[i], fp, 1);
#endif
}

static int compute_tpm()
{
	int t = 0;
	int m = player1.score / 60;	/* minutes x 100 */
	int i;
	for (i=0; i<7; i++)
		t += tetr_stats[i];
	return t*100L / m;
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
	hs.level = player1.level;
	hs.startlvl_ht = player1.height;
	switch (game.mode) {
	case MODE_1P_B:
		hs.lines_tpm = 25;
		hiscores = hiscores_btype;
		break;
	case MODE_1P_40L:
		hs.lines_tpm = compute_tpm();
		hiscores = hiscores_40L;
		break;
	default:
		hs.startlvl_ht = player1.startlevel;
		hs.lines_tpm = player1.lines;
		hiscores = hiscores_atype;
	}
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

void gethiscorestr(int i, char *buf)
{
	struct hiscore *hs = &hiscores[i];
	int secs;
	if (hiscores == hiscores_40L) {
		secs = hs->score/100;
		sprintf(buf, "%d:%02d.%02d", secs/60, secs%60, hs->score%100);
	} else
		sprintf(buf, "%7ld", (long) hs->score);
}

int gethiscorelist(char *s)
{
	char name[8];
	struct hiscore *hs = hiscores;
	int no = 0;
	int i, n;
	char *eol;
	for (i=0; no < 10 && i < num_hiscores; i++, hs++) {
		if (!hs->score)
			continue;
		no++;
		n = sprintf(s, "%2d. %s", no, gethiscorename(i, name));
		memset(s+n, ' ', 12-n);
		s += 12;
		gethiscorestr(i, s);
		s += 7;
		memset(s, ' ', 11);
		s += 2;
		eol = s+9;
		if (hiscores == hiscores_atype) {
			n = sprintf(s, "%d-%d", hs->startlvl_ht, hs->level);
			s[n] = ' ';
			s += 6;
		}
		if (hiscores == hiscores_btype) {
			s++;
			n = sprintf(s, "%d   %d", hs->level, hs->startlvl_ht);
		} else
			n = sprintf(s, "%3d", hs->lines_tpm);
		s[n] = ' ';
		strcpy(eol, no<9 ? " \n" : "\n");
		s = eol+2;
	}
	return no;
}
