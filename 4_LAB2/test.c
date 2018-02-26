#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define N	(5)

int main()
{
	int i;
	int fd[N];
	char buf[64];

	for(i = 0; i < N; i++){
			fd[i] = open("/dev/mycdrv", O_RDWR);
			snprintf(buf, 64, "Count %d", i);
			write(fd[i], buf, strlen(buf) + 1);
			read(fd[i], buf, 64);
			puts(buf);
			close(fd[i]);
	}
}
