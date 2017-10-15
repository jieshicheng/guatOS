#include "shell.h"
#include "stdint.h"
#include "global.h"
#include "debug.h"
#include "stdio.h"
#include "syscall.h"
#include "file.h"
#include "string.h"

#define cmd_len 128
#define MAX_ARG_NR 16

static char cmd_line[cmd_len] = {0};
char cwd_cache[64] = {0};

void print_prompt(void)
{
	printf("[rabbit@localhost %s]$ ", cwd_cache);
}

static void readline(char *buf, int32_t count)
{
	ASSERT(buf != NULL && count > 0);
	char *pos = buf;
	while( read(stdin_no, pos, 1) != -1 && (pos - buf) < count ) {
		switch( *pos ) {
			case '\n':
			case '\r':
				*pos = 0;
				return ;
			case '\b':
				if( buf[0] != '\b' ) {
					--pos;
					putchar('\b');
				}
				break;
			default:
				putchar(*pos);
				++pos;
		}
	}
	printf("readline: can't find entry_key in the cmd_line, max num of char is 128\n");
}

void my_shell(void)
{
	cwd_cache[0] = '/';
	while( 1 ) {
		print_prompt();
		memset(cmd_line, 0, cmd_len);
		readline(cmd_line, cmd_len);
		if( cmd_line[0] == 0 ) {
			continue;
		}
	}
}
