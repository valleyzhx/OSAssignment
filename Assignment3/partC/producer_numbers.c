#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define MAXLEN 100

int main(int argc, char *argv[])
{
    int fd = open("/dev/numpipe", O_RDWR);
	char numstr[MAXLEN];
	int num_to_write;
    if(fd<0){
        printf("open /dev/numpipe Error: %d\n",fd);
        return fd;
    }
    int count=0;
	while(1) {
		bzero(numstr, MAXLEN);
        sprintf(numstr, "%d\n", getpid());
		num_to_write = atoi(numstr);
		// write to pipe
		ssize_t ret = write(fd, &num_to_write, sizeof(int));
		if ( ret < 0) {
			fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
			perror("");
		} else {
            printf("%d produce times:%d, Bytes written: %ld\n", num_to_write,++count,ret);
		}
		sleep(1);
	}

	close(fd);

	return 0;
}

