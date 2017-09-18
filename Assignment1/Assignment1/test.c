//
//  minish.c
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
    ProcessModeForeground = 0,
    ProcessModeBackground = 1,
} ProcessMode;


typedef struct {
    pid_t pid;
    char *status;
    char *commandName;
} ProcessStatus;

ProcessStatus proStatuArray[kLength];
int proIndex = 0;



#pragma mark- run command

int innerCommand(char *command[],int *error){
    
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
        }else{
            *error = 1;
        }
        return 1;
    }
    return 0;
}

int outerCommand(char *command[],int *error){
    int err = execvp(command[0],command);// relative path, file, params
    if (err==-1) {
        *error = 1;
        printf("Run Command Error!\n");
        exit(1);
        
    }
    
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


void runCommand(char *command[],ProcessMode mode){
    
    int error = 0;
    
    if (customCommand(command)) {// check whether custom command
        
    }else{
        outerCommand(command,&error);
    }
    
    //    if (mode == ProcessModeBackground) {
    //        pid_t pid = getpid();
    //        for (int i=0; i<proIndex; i++) {
    //            ProcessStatus proStatus = proStatuArray[i];
    //            if (proStatus.pid == pid) {
    //                proStatus.status = "Finish";
    //                proStatuArray[i] = proStatus;
    //                break;
    //            }
    //        }
    //    }
    
    exit(error);
    
}


#pragma mark- runProcess

void runProcess(char *command[],ProcessMode mode){
    pid_t pid;
    
    int error = 0;
    if (innerCommand(command,&error)) {// inner command, perform directly
        
    }else{
        pid=fork(); // creat process
        if (pid <0) {
            printf("fork() error!");
            exit(1);
        }else if (pid == 0) // child process
        {
            runCommand(command,mode);
            exit(0);
        }else { // father process
            if (mode == ProcessModeBackground) {
                ProcessStatus pro = proStatuArray[proIndex];
                pro.pid = pid;
                proStatuArray[proIndex] = pro;
                proIndex++;
            }
            waitpid(pid,NULL,0);
            
            
        }
        
    }
    
}


#pragma mark- makeCommand
ProcessMode makeCommand(const char *buf,char *command[]){
    
    int wordIndex = 0;
    int letterIndex = 0;
    
    long length = strlen(buf);
    char word[kLength];
    bool isEnd = false;
    ProcessMode mode = ProcessModeForeground;
    
    for (int i=0; i<length; i++) {
        char c = buf[i];//every letter
        if (c == ' ') {// make an word
            c = '\0';
        }else if (c == '\n'){//command end
            c = '\0';
            isEnd = true;
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
    printf("minish>");
    char buf[kLength];
    while (1) {
        //DELog("input: %s",buf);
        fgets(buf,kLength,stdin);
        char *command[kLength];
        ProcessMode mode = makeCommand(buf, command);
        runProcess(command, mode);
        
    }
    
    return 0;
}
