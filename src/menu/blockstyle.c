#include <string.h>
#include "menu.h"
#include "menuext.h"
#include "../textgfx/textgfx.h"
#include "../options.h"

int getblockstyle()
{
	if (_TT_BLOCKS)
		return 1;
	if (_TT_BLOCKS_BG)
		return 2;
	if (textgfx_flags & BLACK_BRACKETS && block_chars[0]=='[' &&
					      block_chars[1]==']')
		return 3;
	if (!getopt_int("term", "block"))
		return 0;
	return 4;
}

static void setblockstyle(int i, int c)
{
	union val v;
	int tp = 0;
	if (_TT_BLOCKS && bgdot==' ') {
		bgdot = default_bgdot();
		unsetoption("term", "bgdot");
	}
	textgfx_flags &= ~(TT_BLOCKS | TT_BLOCKS_BG);
	if (!c) {
		textgfx_flags &= ~BLACK_BRACKETS;
		c = 'X';
	}
	v.integ = 0;
	switch (i) {
	case 0:
		unsetoption("term", "block");
		reset_block_chars();
		return;
	case 1:
		textgfx_flags |= TT_BLOCKS;
		bgdot = ' ';
		setoption("term", "bgdot", v, 1);
		v.integ = -1;
		break;
	case 2:
		textgfx_flags |= TT_BLOCKS_BG;
		v.integ = -2;
		break;
	case 3:
		textgfx_flags |= BLACK_BRACKETS;
		block_chars[0] = '[';
		block_chars[1] = ']';
		strcpy(v.str, "[]b");
		tp = 1;
		break;
	case 4:
		block_chars[0] = c;
		block_chars[1] = ' ';
		v.str[0] = c;
		if (textgfx_flags & BLACK_BRACKETS) {
			v.str[1] = ' ';
			v.str[2] = 'b';
		}
		tp = 1;
	}
	setoption("term", "block", v, tp);
}

static void save_custom_blockstyle()
{
	union val v;
	v.integ = 0;
	v.str[0] = block_chars[0];
	if (v.str[0]==' ')
		v.str[0]--;
	v.str[1] = block_chars[1];
	if (block_chars[0] & 0x100)
		v.str[2] = 'a';
	else if (textgfx_flags & BLACK_BRACKETS)
		v.str[2] = 'b';
	else
		v.str[2] = '_';
	setoption("term", "block", v, 1);
}

static void edit_custom_blockstyle(int i, int c)
{
	static int pos;
	if (i != 4) {
		setblockstyle(4, c);
		pos = 1;
		return;
	}
	if (!pos || pos == 2 && c != 'a') {
		block_chars[0] = c;
		block_chars[1] = ' ';
		pos = 1;
	} else if (pos == 1) {
		block_chars[1] = c;
		pos = (textgfx_flags & (ASCII | BLACK_BRACKETS)) ? 0 : 2;
	} else {
		block_chars[0] |= 0x100;
		block_chars[1] |= 0x100;
		pos = 0;
	}
	save_custom_blockstyle();
}

int select_blockstyle(int k)
{
	const char *styles[5] = {"default","tt","tt-bg","brackets","custom"};
	const char **items = styles;
	int n = 5;
	int i = getblockstyle();
	int j;
	if (isprintable(k)) {
		edit_custom_blockstyle(i, k);
		return 3;
	}
	if ((textgfx_flags & (WHITE_BG | MONOCHROME)) == WHITE_BG) {
		items[1] = items[0];
		items++;
		n--;
		if (i)
			i--;
	}
	j = i;
	k = selectitem(items, n, &i, k);
	if (i != j) {
		if (i && n==4)
			i++;
		setblockstyle(i, 0);
		return 3;
	}
	if (k != 1)
		return k;
	if (textgfx_flags & (TT_BLOCKS | TT_BLOCKS_BG))
		printstr("  ");
	else {
		putch(block_chars[0]);
		putch(block_chars[1]);
	}
	return 1;
}
