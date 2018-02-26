#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE	"/dev/globalmem"

int main(int argc, char *argv[])
{
	int fd;
	void *buf;
	int pagesize = getpagesize();
	int aligned = pagesize;

	posix_memalign(&buf, aligned, pagesize);
	printf("buf: %p\n", buf);

	fd = open(DEVICE, O_RDONLY, S_IRUSR);
	printf("fd: %d\n", fd);

	read(fd, buf, pagesize);
	printf("Read: %s\n", buf);
	
	close(fd);
}
