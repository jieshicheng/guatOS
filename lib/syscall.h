#ifndef __KERNEL_SYSCALL_H
#define __KERNEL_SYSCALL_H

#include "stdint.h"
#include "thread.h"
#include "direct.h"
#include "fs.h"

enum SYSCALL_NR
{
	SYS_GETPID,
	SYS_WRITE,
	SYS_MALLOC,
	SYS_FREE,
	SYS_FORK,
	SYS_READ,
	SYS_CLEAR,
	SYS_PUTCHAR,
	SYS_GETCWD,
	SYS_OPEN,
	SYS_CLOSE,
	SYS_LSEEK,
	SYS_UNLINK,
	SYS_MKDIR,
	SYS_RMDIR,
	SYS_OPENDIR,
	SYS_CHDIR,
	SYS_CLOSEDIR,
	SYS_READDIR,
	SYS_REWINDDIR,
	SYS_STAT,
	SYS_PS,
	SYS_EXECV,
	SYS_WAIT,
	SYS_EXIT
};

uint32_t getpid(void);
uint32_t write(int32_t fd, const void *buf, uint32_t count);
void *malloc(uint32_t size);
void free(void *ptr);
pid_t fork(void);
int32_t read(int32_t fd, void *buf, uint32_t count);
void putchar(char char_asci);
void clear(void);
void ps(void);
int32_t chdir(const char *path);
int32_t stat(const char *path, struct stat *buf);
void rewinddir(struct dir *dir);
struct dir_entry *readdir(struct dir *dir);
int32_t rmdir(const char *pathname);
int32_t closedir(struct dir *dir);
struct dir *opendir(const char *name);
int32_t mkdir(const char *pathname);
int32_t unlink(const char *pathname);
int32_t lseek(int32_t fd, int32_t offset, uint8_t whence);
int32_t close(int32_t fd);
int32_t open(char *pathname, uint8_t flag);
char *getcwd(char *buf, uint32_t size);
int32_t execv(const char *path, const char *argv[]);
void exit(int32_t status);
pid_t wait(int32_t *status);
#endif
