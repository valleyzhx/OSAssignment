//
//  main.c
//  SystemCall
//
//  Created by Xiang on 2017/9/27.
//  Copyright © 2017年 xianng. All rights reserved.
//

#include <stdio.h>
#include<unistd.h>
#include<linux/unistd.h>
#include<linux/time.h>
#include <errno.h>

#define __NR_sys_my_xtime 333

int main(int argc, const char * argv[]) {
    // insert code here...
    struct timespec ts;
    int ret = 0;
    ret = syscall(__NR_sys_my_xtime,&ts);
    printf("ts time:  %.9ld \n,",ts.tv_nsec);
    printf("ret is: %d\n",ret);
    
    
    if (errno) {
        
    }
    
    
    return ret;
    
}
