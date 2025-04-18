#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
int monitor_running = 0;
pid_t monitor_pid;

void list_hunts(int sig)
{
}

void list_treasures(int sig)
{
    printf("\n");
    system("./treasure_manager --list hunt1 ");
}

void view_treasure(int sig)
{
    printf("\n");
    printf("SIGUSR2 received: Executing list_treasures\n");
}

void stop_monitor()
{
    int status;

    if (kill(monitor_pid, SIGTERM) < 0)
    {
        printf("Error sending SIGTERM to child\n");
        exit(2);
    }

    wait(&status);
    printf("Monitor procces ended with code %d\n", WEXITSTATUS(status));
}

void end_monitor_process(int sig)
{
    exit(0);
}

void monitor_procces()
{

    struct sigaction monitor_actions;
    memset(&monitor_actions, 0x00, sizeof(struct sigaction));
    printf("Monitor procces started\n");
    monitor_actions.sa_handler = list_treasures;
    if (sigaction(SIGUSR2, &monitor_actions, NULL) < 0)
    {
        perror("Process a SIGUSR2 sigaction");
        exit(-1);
    }

    monitor_actions.sa_handler = list_hunts;
    if (sigaction(SIGUSR1, &monitor_actions, NULL) < 0)
    {
        perror("Process a SIGUSR1 sigaction");
        exit(-1);
    }

    monitor_actions.sa_handler = view_treasure;
    if (sigaction(SIGILL, &monitor_actions, NULL) < 0)
    {
        perror("Process a SIGILL sigaction");
        exit(-1);
    }

    monitor_actions.sa_handler = end_monitor_process;
    if (sigaction(SIGTERM, &monitor_actions, NULL) < 0)
    {
        perror("Process a SIGTERM sigaction");
        exit(-1);
    }

    while (1)
    {
        pause();
    }
}

int main(void)
{

    char command[100] = "";

    while (1)
    {
        printf("Start monitor\nList hunts\nList treasures\nView treasures\nStop monitor\nExit\nSelect a command:");
        scanf("%s", command);

        if (strcmp(command, "sm") == 0)
        {
            if (monitor_running == 1)
            {
                printf("Monitor is curently running\n");
            }
            else
            {
                monitor_running = 1;
                if ((monitor_pid = fork()) < 0)
                {
                    printf("Error creating child process\n");
                    exit(1);
                }
                if (monitor_pid == 0)
                {
                    monitor_procces();
                    exit(0);
                }
                else
                {
                    sleep(1);
                    printf("\n");
                    continue;
                }
            }
        }
        else if (strcmp(command, "list_hunt") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing list_hunt command\n");
            }
            else
            {

                if (kill(monitor_pid, SIGUSR1) < 0)
                {
                    printf("Error sending SIGUSR to child\n");
                    exit(2);
                }
            }
        }
        else if (strcmp(command, "ls") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing list_treasures command\n");
            }
            else
            {

                if (kill(monitor_pid, SIGUSR2) < 0)
                {
                    printf("Error sending SIGUSR to child\n");
                    exit(2);
                }
                sleep(1);
            }
        }
        else if (strcmp(command, "vt") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing view_treasure command\n");
            }
            else
            {

                if (kill(monitor_pid, SIGILL) < 0)
                {
                    printf("Error sending SIGILL to child\n");
                    exit(2);
                }
                sleep(1);
            }
        }
        else if (strcmp(command, "stm") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Monitor is not currently running");
            }
            else
            {
                stop_monitor();
                monitor_running = 0;
            }
        }
        else if (strcmp(command, "exit") == 0)
        {
            if (monitor_running == 1)
            {
                printf("Monitor running, please close monitor before exiting.\n ");
            }
            else
            {
                exit(0);
            }
        }
        else
        {
            printf("Invalid command");
        }
        printf("\n");
    }
}