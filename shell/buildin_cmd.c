#include "buildin_cmd.h"
#include "debug.h"
#include "direct.h"
#include "string.h"
#include "fs.h"
#include "syscall.h"
#include "stdio.h"

extern char final_path[MAX_PATH_LEN];

static void wash_path(char *old_abs_path, char *new_abs_path)
{
	ASSERT(old_abs_path[0] == '/');
	char name[MAX_FILE_NAME_LEN] = {0};
	char *sub_path = old_abs_path;

	sub_path = path_parse(sub_path, name);
	if( name[0] == 0 ) {
		new_abs_path[0] = '/';
		new_abs_path[1] = 0;
		return ;
	}
	new_abs_path[0] = 0;
	strcat(new_abs_path, "/");
	while( name[0] ) {
		if( !strcmp("..", name) ) {
			char *slash_ptr = strrchr(new_abs_path, '/');
			if( slash_ptr != new_abs_path ) {
				*slash_ptr = 0;
			}
			else {
				*(slash_ptr + 1) = 0;
			}
		}
		else if( strcmp(".", name) ) {
			if( strcmp(new_abs_path, "/") ) {
				strcat(new_abs_path, "/");
			}
			strcat(new_abs_path, name);
		}

		memset(name, 0, MAX_FILE_NAME_LEN);
		if( sub_path ) {
			sub_path = path_parse(sub_path, name);
		}
	}
}

void make_clear_abs_path(char *path, char *final_path)
{
	char abs_path[MAX_PATH_LEN] = {0};
	if( path[0] != '/' ) {
		memset(abs_path, 0, MAX_PATH_LEN);
		if( getcwd(abs_path, MAX_PATH_LEN) != NULL ) {
			if( !((abs_path[0] == '/') && (abs_path[1] == 0)) ) {
				strcat(abs_path, "/");
			}
		}
	}
	strcat(abs_path, path);
	wash_path(abs_path, final_path);
}


void buildin_pwd(uitn32_t argc, char **argv UNUSED)
{
	if( argc != 1 ) {
		printf("pwd: no argument support!\n");
		return ;
	}
	else {
		if( getcwd(final_path, MAX_PATH_LEN) != NULL ) {
			printf("%s\n", final_path);
		}
		else {
			printf("pwd: get current work directory failed.\n");
		}
	}
}

char *buildin_cd(uitn32_t argc, char **argv)
{
	if( argc > 2 ) {
		printf("cd: only support one argument!\n");
		return NULL;
	}
	if( argc == 1 ) {
		final_path[0] = '/';
		final_path[1] = 0;
	}
	else {
		make_clear_abs_path(argv[1], final_path);
	}

	if( chdir(final_path) == -1 ) {
		printf("cd: no such directory %s\n", final_path);
		return NULL;
	}
	return final_path;
}

void buildin_ls(uitn32_t argc, cahr **argv)
{
	char *pathname = NULL;
	struct stat file_stat;
	memset(&file_stat, 0, sizeof(struct stat));
	enum bool long_info = false;
	uitn32_t arg_path_nr = 0;
	uitn32_t arg_idx = 1;
	while( arg_idx < argc ) {
		if( argv[arg_idx][0] == '-' ) {
			if( !strcmp("-l", argv[arg_idx]) ) {
				long_info = true;
			}
			else if( !strcmp("-h", argv[arg_idx]) ) {
				printf("usage: -l list all information about file.\n
					-h for help.\nlist all files in the current directory if no option.\n");
			}
			else {
				printf("ls: invalid option %s\nTry 'ls -h' for help\n", argv[arg_idx]);
				return ;
			}
		}
		else {
			if( arg_path_nr == 0 ) {
				pathname = argv[arg_idx];
				arg_path_nr = 1;
			}
			else {
				printf("ls: only support one path\n");
				return ;
			}
		}
		arg_idx++;
	}

	if( pathname == NULL ) {
		if( getcwd(final_path, MAX_PATH_LEN) != NULL ) {
			pathname = final_path;
		}
		else {
			printf("ls: getcwd for default path failed.\n");
			return ;
		}
	}
	else {
		make_clear_abs_path(pathname, final_path);
		pathname = final_path;
	}

	if( stat(pathname, &file_stat) == -1 ) {
		printf("ls: cannot access %s: No such file or directory.\n", pathname);
		return ;
	}
	if( file_stat.st_filetype == FT_DIRECTORY ) {
		struct dir *dir = opendir(pathname);
		struct dir_entry *dir_e = NULL;
		char sub_pathname[MAX_PATH_LEN] = {0};
		uitn32_t pathname_len = strlen(pathname);
		uitn32_t last_char_idx = pathname_len - 1;
		memcpy(sub_pathname, pathname, pathname_len);
		if( sub_pathname[last_char_idx] != '/' ) {
			sub_pathname[pathname_len] = '/';
			pathname_len++;
		}
		rewinddir(dir);
		if( long_info ) {
			char ftype;
			printf("total: %d\n", file_stat.st_size);
			while( (dir_e = readdir(dir)) ) {
				ftype = 'd';
				if( dir_e->f_type == FT_REGULAR ) {
					ftype = '-';
				}
				sub_pathname[pathname_len] = 0;
				strcat(sub_pathname, dir_e->name);
				memset(&file_stat, 0, sizeof(struct stat));
				if( stat(sub_pathname, &file_stat) == -1 ) {
					printf("ls: cannot access %s: No such file or directory.\n", dir_e->name);
					return ;
				}
				printf("%c  %d  %d  %s\n", ftype, dir_e->i_no, file_stat.st_size, dir_e->name);
			}
		}
		else {
			while( (dir_e = readdir(dir)) ) {
				printf("%s ", dir_e->name);
			}
			printf("\n");
		}
		close(dir);
	}
	else {
		if( long_info ) {
			printf("-  %d  %d  %s\n", file_stat.st_ino, file_stat.st_size, pathname);
		}
		else {
			printf("%s\n", pathname);
		}
	}
}


void buildin_ps(uitn32_t argc, char **argv UNUSED)
{
	if( argc != 1 ) {
		printf("ps: no argument support!\n");
		return ;
	}
	ps();
}

void buildin_clear(uitn32_t argc, char **argv UNUSED)
{
	if( argc != 1 ) {
		printf("clear: no argument support!\n");
		return ;
	}
	clear();
}

int32_t buildin_mkdir(uitn32_t argc, char **argv)
{
	int32_t ret = -1;
	if( argc != 2 ) {
		printf("mkdir: no argument support!\n");
		return ;
	}
	else {
		make_clear_abs_path(argv[1], final_path);
		if( !strcmp("/", final_path) ) {
			if( mkdir(final_path) == 0 ) {
				ret = 0;
			}
			else {
				printf("mkdir: create directory %s failed.\n", argv[1]);
			}
		}
	}
	return ret;
}

int32_t buildin_rmdir(uitn32_t argc, char **argv)
{
	int32_t ret = -1;
	if( argc != 2 ) {
		printf("rmdir: only support 1 argument!\n");
	}
	else {
		make_clear_abs_path(argv[1], final_path);
		if( strcmp("/", final_path) == 0 ) {
			if( rmdir(final_path) == 0 ) {
				ret = 0;
			}
			else {
				printf("rmdir: remove %s failed.\n", argv[1]);
			}
		}
	}
	return ret;
}


int32_t buildin_rm(uitn32_t argc, char **argv)
{
	int32_t ret = -1;
	if( argc != 2 ) {
		printf("rm: only support one argument!\n");
	}
	else {
		make_clear_abs_path(argv[1], final_path);
		if( strcmp("/", final_path) == 0 ) {
			if( unlink(final_path) == 0 ) {
				ret = 0;
			}
			else {
				printf("rm: delete %s failed.\n", argv[1]);
			}
		}
	}
	return ret;
}













