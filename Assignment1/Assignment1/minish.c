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
#include <signal.h>

#define kLength 100

#define INNER 1
#define DEBUG 0
#define DEBUGListJobs 0

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
ProcessStatus array[kLength];
int processIndex = 0;
int latestPid = 0;

#pragma mark- run command

int innerCommand(char *command[],int *error){
    if (strcmp(command[0],"exit\0")==0) {
        for (int i=0; i<processIndex; i++) {
            ProcessStatus proStatus = array[i];
            kill(proStatus.pid, SIGKILL);
            kill(latestPid, SIGKILL);
        }
        
        kill(0, SIGTERM);
        
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
            printf("No Such Directory\n");
        }
        return 1;
    }
    return 0;
}
int outerCommand(char *command[],int *error , ProcessMode mode){
    if (DEBUGListJobs && mode == ProcessModeBackground) {
        sleep(3);
    }
    int err = execvp(command[0],command);// relative path, file, params
    
    if (err==-1) {
        *error = EXIT_FAILURE;
        printf("Run Command Error: %d\n",*error);
    }
    
    return 0;
}
int listJobsAction(char *command[]){
    int returnNum = 0;
    if (strcmp(command[0],"listjobs\0")==0) {
        for (int i=0; i<processIndex; i++) {
            ProcessStatus proStatus = array[i];
            int status = -1;
            waitpid(proStatus.pid,&status,WNOHANG);
            if (status == EXIT_SUCCESS) {
                proStatus.status = "FINISH";
            }else if(status != -1){
                continue;
            }
            array[i] = proStatus;
            printf(" %s with PID %d Status:%s\n",proStatus.commandName,proStatus.pid,proStatus.status);
        }
        returnNum = 1;
    }else if (strcmp(command[0],"fg\0")==0){
        int pid = (atoi)(command[1]);
//        printf("fg -> pid : %d",pid);
        while (waitpid(pid,NULL,WUNTRACED)!=pid) {
            
        }
        returnNum = 1;

    }
    return returnNum;
}


void runCommand(char *command[],ProcessMode mode){
    int error = EXIT_SUCCESS;
    outerCommand(command,&error,mode);
    exit(error);
}
#pragma mark- runProcess
void runProcess(char *command[],ProcessMode mode,ProcessStatus *pro){
    pid_t pid;
    int error = 0;
    if (!innerCommand(command, &error)) {//if inner command, perform directly , else =>
        if (!listJobsAction(command)) { // if not listjobs
            pid = fork();
            if (pid <0) {
                printf("fork() error!");
                exit(EXIT_FAILURE);
            }
            int status;
            if (pid == 0) { //child process
                runCommand(command,mode);
            }else{//father process

                if (mode == ProcessModeForeground) {// foreground mode, should wait
                    latestPid = pid;
                    while (waitpid(pid,&status,0)!=pid);
                }else{
                    pro->pid = pid;
                    array[processIndex] = *pro;
                    processIndex++;
                }
            }
        }
    }
    
    
}
#pragma mark- makeCommand
ProcessMode makeCommand(const char *buf,char *command[],ProcessStatus *proStatus){
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
        if (c == '&' && buf[i-1]==' '&&buf[i+1]=='\n') {
            
            c = '\0';
            command[wordIndex] = NULL;
            
            char *com = malloc(sizeof(char) * kLength);
            strncpy(com, buf,length-1);
            com[length-1] = '\0';
            
            ProcessStatus pro = {0,"RUNNING", com};
            *proStatus = pro;
            mode = ProcessModeBackground;
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

void intHandler(int dummy){
    DELog("dummy: %d",dummy)
    kill(latestPid, SIGKILL);
}
#pragma mark- main

int main(int argc, const char * argv[]) {
    signal(SIGINT, intHandler);
    char buf[kLength];
    while (1) {
        printf("minish>");
        fgets(buf,kLength,stdin);
        char *command[kLength];
        ProcessStatus pro;
        ProcessMode mode = makeCommand(buf, command,&pro);
        runProcess(command, mode,&pro);
        
    }
    return 0;
}
