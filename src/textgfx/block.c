#include <string.h>
#include <stdio.h>
#include "textgfx.h"
#include "../options.h"

short block_chars[2] = {TEXTURE1, TEXTURE2};
short bgdot = 0;

static void sprint_num(char *s, int i)
{
#if __STDC_VERSION__ >= 199901L
	snprintf(s, 3, "%d", i);
	s[3] = '\0';
#else
	if (i < 0) {
		*s = '-';
		s++;
		i = -i;
	}
	while (i > 99)
		i /= 10;
	if (i > 9) {
		*s = (i /= 10) + '0';
		s++;
	}
	s[0] = i+'0';
	s[1] = '\0';
#endif
}

void blockstyle_from_option(const struct option *o)
{
	const char *k = opt_key(o);
	union val v = o->val;
	int i;
#if !NO_BLOCKSTYLES
	if (!strcmp(k, "block")) {
		if (opt_isint(o)) {
			if (v.integ == -1) {
				textgfx_flags |= TT_BLOCKS;
				return;
			}
			if (v.integ == -2) {
				textgfx_flags |= TT_BLOCKS_BG;
				return;
			}
			sprint_num(v.str, v.integ);
		}
		for (i = 0; i < 2; i++) {
			block_chars[i] = isprintable(v.str[i])
						? v.str[i] : ' ';
			if (v.str[2] == 'a')
				block_chars[i] |= 0x100;
		}
		if (v.str[2] == 'b')
			textgfx_flags |= BLACK_BRACKETS;
	} else
#endif
	if (!strcmp(k, "bgdot")) {
		if (v.str[0] == '~')
			bgdot = BULLET;
		else if (isprintable(v.str[0]))
			bgdot = v.str[0];
		else
			bgdot = ' ';
	}
}

static void setblockchars_tt(int clr)
{
	int c = 0;
	switch (clr) {
	case 1:
		block_chars[0] = '<';
		block_chars[1] = '>';
		break;
	case 2:
		c = '%';
		break;
	case 3:
		c = '#';
		break;
	case 4:
		block_chars[0] = '[';
		block_chars[1] = ']';
		break;
	case 5:
		block_chars[0] = '(';
		block_chars[1] = ')';
		break;
	case 6:
		c = '@';
		break;
	case 7:
		block_chars[0] = '{';
		block_chars[1] = '}';
	}
	if (c) {
		block_chars[0] = c;
		block_chars[1] = c;
	}
}

void setblockcolor(int clr)
{
	if (!clr) {
		setattr_normal();
		if (bgdot != ' ')
			set_color_pair(BOARD_BG_COLOR);
		return;
	}
	if (!_MONOCHROME)
		set_color_pair(clr);
	else
#if !NO_BLOCKSTYLES
	if (!_TT_BLOCKS)
#endif
	{
		setattr_normal();
		setattr_standout();
	}
#if !NO_BLOCKSTYLES
	if (textgfx_flags & (TT_BLOCKS | TT_BLOCKS_BG))
		setblockchars_tt(clr);
#endif
}

void reset_block_chars()
{
#if !NO_BLOCKSTYLES
	if (getopt_int("term", "block"))
		return;
#endif
	if ((textgfx_flags & (MONOCHROME | ASCII))==MONOCHROME) {
		block_chars[0] = TEXTURE2;
		block_chars[1] = TEXTURE1;
	} else {
		block_chars[0] = TEXTURE1;
		block_chars[1] = TEXTURE2;
	}
}
