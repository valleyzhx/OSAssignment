#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLEN 100

int main(int argc, char *argv[])
{

    int fd = open("/dev/numpipe", O_RDWR);
	int num;

    if(fd<0){
        printf("open /dev/numpipe Error: %d\n",fd);
        return fd;
    }
    int count = 0;
	while(1) {
		// read a line
		ssize_t ret = read(fd, &num, sizeof(int));
		if( ret > 0) {
			printf("%d consume times: %d, Bytes read: %ld\n", getpid(),++count,ret);
		} else {
			fprintf(stderr, "error reading ret=%ld errno=%d perror: ", ret, errno);
			perror("");
			sleep(1);
		}
	}
	close(fd);

	return 0;
}

