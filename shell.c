#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<signal.h>
#include<string.h>

static char* history_command[1024];
static int argc = 0;

void execute_command(char* line);
void add_to_history(char* command);
void run_from_history();

void main(void)
{
    char line[1024];
    int should_run = 1;

    while (should_run)
    {
        argc = 0;
        printf("osh> ");
        gets(line);

        if(strcmp(line, "!!") == 0)
        {
            run_from_history();
        }
        else if(strcmp(line, "exit") == 0)
        {
            exit(0);
        }
        else
        {
            add_to_history(line);
            execute_command(line);
        }
        printf("\n");
    }
}

void execute_command(char* line)
{
    char* argv[64];

    while (*line!='\0')
    {
        while (*line == ' ' || *line == '\t' || *line == '\n')
        {
            *line = '\0';
            line++;
        }
        argv[argc] = line;
        argc++;

        while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n')
        {
            line++;
        }
    }
    argv[argc] = NULL;

    pid_t new_pid;
    int child_status;
    int is_concurrent = 0;
    new_pid = fork();

    if(strcmp(argv[argc-1], "&") == 0)
    {
        is_concurrent = 1;
        argv[argc-1] = '\0';
    }
    
    switch (new_pid)
    {
    case -1:
        printf("ERROR: Cannot fork a child process.");
        exit(1);
        break;
    case 0:
        if(execvp(*argv, argv) < 0)
        {
            printf("ERROR: Cannot execute command");
            exit(1);
        }
        else
        {
            //Command is being executed
        }
        break;
    default:
        if(is_concurrent == 0)
        {
            wait(&child_status);
        }
        else
        {
            
        }
        break;
    }
}

void add_to_history(char* command)
{
    history_command[0] = strdup(command);
}

void run_from_history()
{
    execute_command(history_command[0]);
}