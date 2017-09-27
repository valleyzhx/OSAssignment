//
//  main.c
//  SystemCall
//
//  Created by Xiang on 2017/9/27.
//  Copyright © 2017年 xianng. All rights reserved.
//

#include <stdio.h>
#include <linux/kernel.h>

long my_xtime(struct timespec *current_time){
    
    printk(KERN_ALERT "Hello World! %d\n", count++);
    return current_time->tv_nsec;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
