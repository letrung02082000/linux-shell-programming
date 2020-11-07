#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<signal.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include <unistd.h>

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

    //Redirect
    //If > then redirect stdout
    //If < then redirect stdin
    int fd[2] = {-1, -1};
    int splitIndex;
    int usePipe = 0;

    for(int i=0; i<argc; i++)
    {
        if(strcmp(argv[i], "|") == 0)
        {
            usePipe = 1;
            splitIndex = i;
            break;
        }
    }

    if(usePipe == 0)
    {
        for(int i=0; i<argc; i++)
        {
            if(strcmp(argv[i], ">") == 0)
            {
                fd[1] = open(argv[i+1], O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP|S_IWGRP);
                
                if(fd[1] == -1 || argv[i+1] == NULL)
                {
                    printf("ERROR: Cannot open file");
                    return;
                }

                argv[i] = NULL;
                break;
            }
            else if(strcmp(argv[i], "<") == 0)
            {
                fd[0] = open(argv[i+1], O_RDONLY);

                if(fd[0] == -1)
                {
                    printf("ERROR: Cannot open file");
                    return;
                }
                argv[i] = NULL;
                break;
            }
        }
    }

    pid_t new_pid;
    int child_status;
    int is_concurrent = 0;
    new_pid = fork();

    if(strcmp(argv[argc-1], "&") == 0)
    {
        is_concurrent = 1;
        argv[argc-1] = NULL;
    }
    
    switch (new_pid)
    {
    case -1:
        printf("ERROR: Cannot fork a child process.");
        exit(1);
        break;
    case 0:
        if(usePipe == 1)
        {
            //Init pipe
            int pipes[2];
            if(pipe(pipes) == -1)
            {
                printf("ERROR: Cannot init a pipe");
                break;
            }

            //Split argv array into 2 arrays equal to 2 commands
            int argc1 = splitIndex + 1;
            int argc2 = argc - splitIndex;
            char* command1[argc1];
            char* command2[argc2];

            for (size_t i = 0; i < splitIndex; i++)
            {
                command1[i] = argv[i];
            }
            command1[argc1 - 1] = NULL;

            for (size_t i = 0; i < (argc2 - 1); i++)
            {
                command2[i] = argv[splitIndex + i + 1];
            }
            command2[argc2 - 1] = NULL;
            
            //Create a child process to execute second command
            pid_t pipe_pid = fork();

            if(pipe_pid > 0)
            {
                wait(NULL);
                close(pipes[1]); //close the write pipe
                dup2(pipes[0], STDIN_FILENO);
                close(pipes[0]);

                if(execvp(command2[0], command2) == -1)
                {
                    printf("ERROR: Cannot execute command 2");
                    exit(1);
                }
                else
                {
                    //Command is being executed
                }
            }
            else if(pipe_pid == 0)
            {
                close(pipes[0]); //close the read pipe
                dup2(pipes[1], STDOUT_FILENO);
                close(pipes[1]);

                if(execvp(command1[0], command1) == -1)
                {
                    printf("ERROR: Cannot execute command 1");
                    exit(1);
                }
                else
                {
                    //Command is being executed
                }
            }
            close(pipes[0]);
            close(pipes[1]);
        }
        else if(usePipe == 0)
        {
            if(fd[0] != -1)
            {
                if(dup2(fd[0], STDIN_FILENO) != STDIN_FILENO)
                {
                    printf("ERROR: Cannot redirect the output");
                    return;
                }
            }
            else if(fd[1] != -1)
            {
                if(dup2(fd[1], STDOUT_FILENO) != STDOUT_FILENO)
                {
                    printf("ERROR: Cannot redirect the output");
                    return;
                }
            }

            if(execvp(argv[0], argv) < 0)
            {
                printf("ERROR: Cannot execute command");
                exit(1);
            }
            else
            {
                //Command is being executed
            }
            break;
        }
        
    default:
        close(fd[0]);
        close(fd[1]);

        if(is_concurrent == 0)
        {
            waitpid(new_pid, &child_status, 0);
        }
        else
        {
            printf("Command is running concurrently");
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