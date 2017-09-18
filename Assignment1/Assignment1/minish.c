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

ProcessStatus *array;


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

int outerCommand(char *command[],int *error , ProcessMode mode){
    if (DEBUGListJobs && mode == ProcessModeBackground) {
        sleep(3);
    }
    int err = execvp(command[0],command);// relative path, file, params
    
    if (err==-1) {
        *error = 1;
        printf("Run Command Error!\n");
    }

    return 0;
}

int customCommand(char *command[]){
    
    if (strcmp(command[0],"listjobs\0")==0) {
        DELog("listjobs aready == %d",555);
        exit(555);
        return 1;
    }
    return 0;
}


void runCommand(char *command[],ProcessMode mode){
    
    int error = 0;
    
    if (customCommand(command)) {// check whether custom command
        
    }else{
        outerCommand(command,&error,mode);
    }
    
    
    exit(error);
    
}


#pragma mark- runProcess

void runProcess(char *command[],ProcessMode mode,ProcessStatus *pro){
    pid_t pid;
    
    int error = 0;
    if (innerCommand(command,&error)) {// inner command, perform directly
        
    }else{
        
        if (mode == ProcessModeBackground) {
            //pipe(fd);
        }
        
        
        pid=fork(); // creat process
        if (pid <0) {
            printf("fork() error!");
            exit(1);
        }else if (pid == 0) // child process
        {
            runCommand(command,mode);
            
        }else { // father process
            if (mode == ProcessModeBackground) {
                pro->pid = pid;
                DELog("new pid:%d %s",pro->pid,pro->commandName);
            }
            
        }
        int status;
        if (mode == ProcessModeForeground) {
            waitpid(pid,&status,0);
        }
        if (mode == ProcessModeBackground) {
            
            int length = sizeof(array)/sizeof(array[0]);
            
            for (int i=0; i<length; i++) {
                ProcessStatus proStatus = array[i];
                DELog("run :%d , name: %s\n",proStatus.pid,proStatus.commandName);
                if (proStatus.pid == pid) {
                    proStatus.status = "Finish";
                    array[i] = proStatus;
                    break;
                }
            }
            if (length == 0) {
                ProcessStatus arr[]={*pro};
                array = arr;
            }
            
            
        }else if (status == 555){
            DELog("end 555!%s\n","sss");
            int length = sizeof(array)/sizeof(array[0]);
            if (length>0) {
                printf("List of backgrounded processes:\n");
                
            }
            for (int i=0; i<length; i++) {
                ProcessStatus proStatus = array[i];
                if (proStatus.pid == 0) {
                    break;
                }
                printf(" %s with PID %d Status:%s\n",proStatus.commandName,proStatus.pid,proStatus.status);
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
            
            char *com = malloc(sizeof(char) * kLength);
            strncpy(com, buf,length-1);
            com[length-1] = '\0';
            
            ProcessStatus pro = {0,"RUNNING", com};
            *proStatus = pro;
            mode = ProcessModeBackground;
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
        ProcessStatus pro;
        ProcessMode mode = makeCommand(buf, command,&pro);
        runProcess(command, mode,&pro);
        
    }
    
    return 0;
}
