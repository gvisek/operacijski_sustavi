#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdatomic.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <string.h>
#include <wait.h>

int pid = -1;

void processInt(int sig){
    if(pid != -1)
        kill(pid, SIGINT);
}

char **getTokens(char *lineptr, int nchars_read, char *delim){
    
    int numTokens = 0;
    char *token = NULL;
    char **argv = NULL;
    char *lineptr_copy = NULL;
    

    if(nchars_read != -1){
        lineptr_copy = malloc(sizeof(char) * nchars_read);
    
        strcpy(lineptr_copy, lineptr);

        token = strtok(lineptr, delim);
        

        while (token != NULL){
            numTokens++;
            token = strtok(NULL, delim);
        }
        numTokens++;
        
        argv = malloc(sizeof(char *) * numTokens);

        token = strtok(lineptr_copy, delim);

        for (int i = 0; token != NULL; i++){
            argv[i] = malloc(sizeof(char) * strlen(token));
            strcpy(argv[i], token);

            token = strtok(NULL, delim);
        }
        argv[numTokens] = NULL;

        free(lineptr_copy);
    }
    
    return argv;
}

char *getPath(char *command){
        

    char *path, *pathCopy;
    path = getenv("PATH");
    pathCopy = strdup(path);
    int commandLenght = strlen(command);
    char *pathToken = strtok(pathCopy, ":");
    char *filePath;
    int directoryLenght;
    struct stat buffer;
    while(pathToken != NULL){
        directoryLenght = strlen(pathToken);
        filePath = malloc(commandLenght + directoryLenght + 2);
        strcpy(filePath, pathToken);
        strcat(filePath, "/");
        strcat(filePath, command);
        strcat(filePath, "\0");
        if(stat(filePath, &buffer) == 0){
            free(pathCopy);
            return (filePath);
        }else{
            free(filePath);
            pathToken = strtok(NULL, ":");

            }

    }
    free(pathCopy);
    if(stat(command, &buffer) == 0){
        return command;
    }
    return NULL;
}


void execute(char **argv){
    

    if(argv){
        char *command = getPath(argv[0]);
    
        if(execve(command, argv, NULL) == -1){
            fprintf(stderr, "unknown command %s\n", argv[0]);
        }
    }
}

int main(int arg, char **argv){
    char *prompt = "fsh>";
    char *lineptr = NULL;
    size_t n = 0; 
    ssize_t nchars_read;
    
    

    struct sigaction act;
    act.sa_handler = processInt;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);
    lineptr = malloc(sizeof(char) * 1024);
    
    while(1){
        
        printf("%s", prompt);
        fflush(stdout);
        
        if(fgets(lineptr, 1024, stdin) == NULL){
            printf("\n");
            lineptr[0] = 0;
            fflush(stdin);
            continue;
        };
        //nchars_read = getline(&lineptr, &n, stdin);
        nchars_read = strlen(lineptr)/sizeof(char);
        
        
        // if(nchars_read == -1){
        //     exit(1);
        // }
        
        if(strlen(lineptr) > 0){    
        if(strcmp(lineptr, "\n") == 0 || strcmp(lineptr, "exit\n") == 0){
            free(lineptr);
            exit(0);
        }
        
        argv = getTokens(lineptr, nchars_read, " \n");
        
        
            
            
        
        if(strcmp(argv[0], "cd") == 0){
            if(chdir(argv[1]) == -1){
                fprintf(stderr, "The directory %s does not exist\n", argv[1]);
            }
            continue;
        }
        pid = fork();
        if(pid == 0){
            if(setpgid(pid, pid) == -1){
                printf("setpgid error\n");
            }
                execute(argv);
        }else{
                wait(NULL);
                pid = -1;
        }
        }else{
            free(lineptr);
            lineptr = NULL;
            n = 0;
            printf("\n");
        }
        
    }

        
    
    free(lineptr); 
    return 0;
}