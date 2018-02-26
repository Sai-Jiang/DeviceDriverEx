#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define N	(5)

int main()
{
	int i;
	int fd[N];

	for(i = 0; i < N; i++){
			fd[i] = open("/dev/mycdrv", O_RDONLY);
	}

	sleep(5);

	for(i = 0; i < N; i++)
		close(fd[i]);
}