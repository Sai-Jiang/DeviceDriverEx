#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#define GET_SIZE 1

void read_and_compare(int fd, char *read_buf, char *mmap_buf, unsigned long len)
{
	/* Read the file and compare with mmap_buf[] */

	if (read(fd, read_buf, len) != len) {
		fprintf(stderr, "read problem:  %s\n", strerror(errno));
		exit(1);
	}

	if (memcmp(read_buf, mmap_buf, len) != 0) {
		fprintf(stderr, "buffer miscompare\n");
		exit(1);
	}
}

int main(int argc,char *argv[])
{
	int fd, i, len;
	char *read_buf, *mmap_buf;
	char *filename = "/dev/globalmem";
	
	if ((fd = open(filename, O_RDWR)) < 0) {
		fprintf(stderr, "open of %s failed:  %s\n", filename,
			strerror(errno));
		exit(1);
	}

	if (ioctl(fd, GET_SIZE, &len) < 0) {
		fprintf(stderr, "ioctl failed:  %s\n", strerror(errno));
		exit(1);
	}
	printf("driver's ioctl says buffer size is %ld\n", len);

	mmap_buf = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mmap_buf == (char *)MAP_FAILED) {
		fprintf(stderr, "mmap of %s failed:  %s\n", filename,
			strerror(errno));
		exit(1);
	}
	printf("mmap succeeded:  %p\n", mmap_buf);

	for (i = 0; i < len; i++)
		*(mmap_buf + i) = (char)i;


	read_buf = malloc(len);

	read_and_compare(fd, read_buf, mmap_buf, len);

	i = random() % len;
	*(mmap_buf + i) = random() % i;

	lseek(fd, 0, SEEK_SET);

	read_and_compare(fd, read_buf, mmap_buf, len);

	printf("comparison of modified data via read() and mmap() successful\n");

	return 0;
}
