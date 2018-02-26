#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define DEVICE	"/dev/globalmem"
#define STRING	"THIS IS A TESTING STRING"	

int main(int argc, char *argv[])
{
	int fd;
	void *buf = NULL;
	int aligned = getpagesize();
	posix_memalign(&buf, aligned, sizeof(STRING));
	strcpy(buf, STRING);

	printf("Buf: %p\n", buf);
	printf("%s\n", buf);

	fd = open(DEVICE, O_WRONLY | O_TRUNC, S_IWUSR);
	printf("fd: %d\n", fd);

	write(fd, buf, sizeof(STRING));
	close(fd);
}
