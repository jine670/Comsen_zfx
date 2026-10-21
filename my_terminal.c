#include "my_terminal.h"

/* 屏幕上 32 个字符的“显存”，以及光标位置 */
static char    screen[MY_TERM_ROWS][MY_TERM_COLS];
static uint8_t cur_col;
static uint8_t cur_row;

static void fill_spaces(char *p, uint8_t n)
{
    uint8_t i;
    for (i = 0U; i < n; i++)
    {
        p[i] = ' ';
    }
}

/* 整屏上滚一行：第 2 行搬到第 1 行，第 2 行清空，光标停在第 2 行行首 */
static void scroll_up(void)
{
    uint8_t c;
    for (c = 0U; c < MY_TERM_COLS; c++)
    {
        screen[0][c] = screen[1][c];
    }
    fill_spaces(&screen[1][0], MY_TERM_COLS);
    cur_row = (uint8_t)(MY_TERM_ROWS - 1U);
    cur_col = 0U;
}

/* 换行：还没到最后一行就往下走，已经是最后一行就滚动 */
static void newline(void)
{
    if ((uint8_t)(cur_row + 1U) < MY_TERM_ROWS)
    {
        cur_row++;
        cur_col = 0U;
    }
    else
    {
        scroll_up();
    }
}

void my_terminal_init(void)
{
    uint8_t r;
    for (r = 0U; r < MY_TERM_ROWS; r++)
    {
        fill_spaces(&screen[r][0], MY_TERM_COLS);
    }
    cur_row = 0U;
    cur_col = 0U;
}

void my_terminal_clear(void)
{
    my_terminal_init();
}

void my_terminal_feed(uint8_t ch)
{
    if (ch == '\r')
    {
        cur_col = 0U; /* 回到本行行首（不换行） */
        return;
    }

    if (ch == '\n')
    {
        newline();
        return;
    }

    if ((ch == '\b') || (ch == 0x7FU)) /* 退格 / DEL */
    {
        if (cur_col > 0U)
        {
            cur_col--;
            screen[cur_row][cur_col] = ' ';
        }
        else if (cur_row > 0U) /* 行首退格：退回上一行末尾 */
        {
            cur_row--;
            cur_col = (uint8_t)(MY_TERM_COLS - 1U);
            screen[cur_row][cur_col] = ' ';
        }
        return;
    }

    if ((ch < 0x20U) || (ch > 0x7EU))
    {
        return; /* 控制符 / 中文 UTF-8 的字节一律丢弃，避免乱码 */
    }

    screen[cur_row][cur_col] = (char)ch;
    cur_col++;
    if (cur_col >= MY_TERM_COLS)
    {
        newline(); /* 一行写满自动换行（到最后一行就滚动） */
    }
}

void my_terminal_puts(const char *s)
{
    while (*s != '\0')
    {
        my_terminal_feed((uint8_t)*s);
        s++;
    }
}

void my_terminal_lines(char line1[16], char line2[16])
{
    uint8_t i;
    for (i = 0U; i < MY_TERM_COLS; i++)
    {
        line1[i] = screen[0][i];
        line2[i] = screen[1][i];
    }
}
