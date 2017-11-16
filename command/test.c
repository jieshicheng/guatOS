#include "stdio.h"
#include "syscall.h"
int main(void) 
{
	int fd = open("/file1", O_RDONLY);
	void *buf = malloc(1024);
	read(fd, buf, 1024);
	write(1, buf, 1024);
	printf("prog_no_can use! %s\n", buf);
	free(buf);
	close(fd);
	return 0;
}
