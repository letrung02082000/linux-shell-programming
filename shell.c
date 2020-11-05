#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<signal.h>
#include<string.h>

static char* history_command[1024];
static int argc = 0;

void split_string(char* line, char** argv);
void execute_command(char** argv);
void add_to_history(char* command);
void run_from_history();

void main(void)
{
    char line[1024];
    char *argv[64];
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
        else
        {
            add_to_history(line);
            split_string(line, argv);

            if(strcmp(argv[0], "exit") == 0)
            {
                exit(0);
            }
            execute_command(argv);
        }
        

        
    }
}

void split_string(char* line, char** argv)
{
    while (*line!='\0')
    {
        while (*line == ' ' || *line == '\t' || *line == '\n')
        {
            *line = '\0';
            line++;
        }
        *argv = line;
        argv++;
        argc++;

        while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n')
        {
            line++;
        }

        *argv = '\0';

    }
}

void execute_command(char** argv)
{
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
    char* argv[64];

    split_string(history_command[0],  argv);
    execute_command(argv);
}