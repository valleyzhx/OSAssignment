//
//  main.c
//  Assignment1
//
//  Created by Xiang on 2017/9/16.
//  Copyright © 2017年 xianng. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>


#define kLength 100

#define INNER 1
#define DEBUG 1
#define DEBUGListJobs 1

#if DEBUG
#define DELog(fmt,...) printf("\n=== DEBUG Print ===\n"); printf(fmt, __VA_ARGS__); printf("\n=== DEBUG End Print ===\n");
#else
#define DELog(fmt,...)
#endif

typedef enum {
    ThreadModeForeground = 0,
    ThreadModeBackground = 1,
} ThreadMode;


typedef struct {
    pid_t pid;
    char *status;
    char *commandName;
} ProcessStatus;

ProcessStatus proStatuArray[kLength];
int proIndex = 0;



#pragma mark- run command

int innerCommand(char *command[]){
    
    if (strcmp(command[0],"exit\0")==0) {
        exit(0);
        return 1;
    }else if (strcmp(command[0],"pwd\0")==0){
        char result[kLength];
        getcwd(result,sizeof(result));//get current dir
        printf("dir:%s\n",result);
        return 1;
    }else if (strcmp(command[0],"cd")==0){//cd dir
        const char *dir = command[1];
        DELog("dir for cd is %s",dir);
        if (chdir(dir)>=0) {
            char result[kLength];
            getcwd(result,sizeof(result));//get current dir
            printf("CD dir: %s\n",result);
        }
        return 1;
    }
    return 0;
}

int outerCommand(char *command[]){
    
    int error = execvp(command[0],command);// relative path, file, params
    if (error==-1) {
        printf("Run Command Error!\n");
        exit(0);
    }
    exit(1);
    
    return 0;
}

int customCommand(char *command[]){
    
    if (strcmp(command[0],"listjobs\0")==0) {
        if (proIndex >0) {
            for (int i=0; i<proIndex; i++) {
                ProcessStatus proStatus = proStatuArray[i];
                printf(" %s with PID %d Status:%s\n",proStatus.commandName,proStatus.pid,proStatus.status);
            }
        }
        
        return 1;
    }
    return 0;
}


void runCommand(char *command[]){
    if (customCommand(command)) {// check whether custom command
        exit(1);
    }else{
        outerCommand(command);//执行
    }
}



#pragma mark- thread fucntion

void* threadMethod(void* arg){
    char **command = (char**)arg;
    DELog("threadId: %ld",(long)pthread_self());
    runCommand(command);
    pthread_exit(0);
}


#pragma mark- runProcess

void runProcess(char *command[],ThreadMode mode){
    pid_t pid;
    
    int inner = 0;
    if (INNER) {
        inner = innerCommand(command);
    }
    if (inner == 0) {
        pid=fork(); // creat process
        if (pid == 0) // child process
        {
            
            if (mode == ThreadModeBackground) {
                pthread_t  threadId;
                pthread_create(&threadId, NULL, threadMethod, command);
            }else{
                runCommand(command);
            }
            
        }else { // father process
            if (mode == ThreadModeBackground) {
                ProcessStatus pro = proStatuArray[proIndex];
                pro.pid = pid;
                proStatuArray[proIndex] = pro;
                proIndex++;
            }
            
            DELog("this is the Parent: wait for pid : %d",pid);
            int status;
            while (waitpid(pid,&status,0) != pid);
            DELog("pid %d status : %d",pid,status);
            
            for (int i=0; i<proIndex; i++) {
                ProcessStatus proStatus = proStatuArray[i];
                //int delete = -1;
                if (proStatus.pid == pid) {
                    if (status == 0) {
                        //delete = i;
                        proStatus.status = "Failure";
                    }else if (status == 1){
                        proStatus.status = "Finish";
                    }
                    proStatuArray[i] = proStatus;
                    
                    //                    if (delete>=0) {
                    //                        proIndex--;
                    //                        int j=0;
                    //                        while (j<=proIndex) {
                    //                            if (j==proIndex) {
                    //                                ProcessStatus sta = {0,NULL,NULL};
                    //                                proStatuArray[j]= sta;
                    //                            }else if (j>=delete) {
                    //                                proStatuArray[j]=proStatuArray[j+1];
                    //                            }
                    //
                    //                        }
                    //                    }
                    
                    break;
                }
            }
            
            
        }
    }
}






#pragma mark- makeCommand
ThreadMode makeCommand(const char *buf,char *command[]){
    
    int wordIndex = 0;
    int letterIndex = 0;
    
    long length = strlen(buf);
    char word[kLength];
    bool isEnd = false;
    ThreadMode mode = ThreadModeForeground;
    
    for (int i=0; i<length; i++) {
        char c = buf[i];//every letter
        if (c == ' ') {// make an word
            c = '\0';
        }else if (c == '\n'){//command end
            c = '\0';
            isEnd = true;
        }
        if (c == '&' && buf[i-1]==' '&&buf[i+1]=='\n') {
            mode = ThreadModeBackground;
            c = '\0';
            command[wordIndex] = NULL;
            
            char *com = malloc(sizeof(char) * kLength);
            strncpy(com, buf,length-1);
            com[length-1] = '\0';
            
            ProcessStatus proStatus = {0,"RUNNING", com};
            proStatuArray[proIndex] = proStatus;
            
            return mode;
        }
        
        word[letterIndex] = c;
        letterIndex++;
        
        
        if (c == '\0') {
            char *copyWord = malloc(strlen(word)+1);// every word end
            strcpy(copyWord, word);
            command[wordIndex] = copyWord;
            
            memset(word,0,kLength);
            letterIndex = 0;
            wordIndex++;
            
            if (isEnd) { // command end
                command[wordIndex] = NULL;
                return mode;
            }
        }
    }
    
    return mode;
}

#pragma mark- main

int main(int argc, const char * argv[]) {
    char buf[kLength];
    while (1) {
        fgets(buf,kLength,stdin);
        char *command[kLength];
        ThreadMode mode = makeCommand(buf, command);
        runProcess(command, mode);
        
    }
    
    return 0;
}
