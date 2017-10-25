#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define N 3
#define MAX_LENGTH 200

struct timeval gtodTimes[N];
char *procClockTime[N];

int main(){
    
    int fd = open("/dev/mytime", O_RDONLY);
    /* check for errors HERE */
    if(fd<0){
        printf("open /dev/mytime Error: %d\n",fd);
        return fd;
    }
    
    for(int i=0; i < N; i++)
    {
        gettimeofday(&gtodTimes[i], 0);
        
        procClockTime[i] =  malloc((sizeof(char))*MAX_LENGTH);
        int bytes_read = read(fd, procClockTime[i], MAX_LENGTH);
        /* check for errors HERE */
        if (bytes_read <0) {
            printf("read /dev/mytime Error: %d\n",bytes_read);
            return bytes_read;
        }
    }
    
    close(fd);
    
    for(int i=0; i < N; i++) {
        /* fix the output format appropriately in the above line */
        struct timeval time = gtodTimes[i];
        printf("user-leve gettimeofday:%9ld %6ld\n%s\n",time.tv_sec,time.tv_usec, procClockTime[i]);
    }
    return 0;
}


