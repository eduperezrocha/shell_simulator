#include "command_line.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 512

void sigchldHandler(int sigID) {
    int status;

    while (waitpid(-1, &status, WNOHANG) > 0) {
        if (WIFEXITED(status)) {
        printf("Exit status: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Exit signal: %d\n", WTERMSIG(status));
    }
    }
}

int main(int argc, const char **argv)
{

    struct sigaction action4SigInt = {0};

    action4SigInt.sa_handler = SIG_IGN;
    sigaction(SIGINT, &action4SigInt, NULL);

    struct sigaction action4SigChld = {0};
    action4SigChld.sa_handler = sigchldHandler;

    char cmdline[MAX_LINE_LENGTH];
    struct CommandLine command;
    for (;;)
    {
        printf("> ");
        fgets(cmdline, MAX_LINE_LENGTH, stdin);
        if (feof(stdin)) 
        {
            exit(0);
        }

        bool gotLine = parseLine(&command, cmdline);
        if (gotLine) 
        {

            if (strcmp(command.arguments[0], "exit") == 0) {
                exit(0);
            } else if (strcmp(command.arguments[0], "cd") == 0) {
                chdir(command.arguments[1]);
            } else {
                pid_t pid = fork();

                int status;
                
                if (pid==0) { //child process
                    action4SigInt.sa_handler = SIG_DFL;
                    sigaction(SIGINT, &action4SigInt, NULL);

                    execvp(command.arguments[0], command.arguments);
                } else if (pid>0) { //parent process
                    
                    if (!command.background) {
                        waitpid(pid, &status, 0);
                    } else {
                        sigaction(SIGCHLD, &action4SigChld, NULL);
                    }
                }
            }
            freeCommand(&command);
        }

    }
}

