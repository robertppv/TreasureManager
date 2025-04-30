#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
int monitor_running = 0;
int monitor_stopping = 0;
pid_t monitor_pid;
mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
#define DEFAULT_LENGTH 32

void fork_exec(char **args)
{
    int exec_pid;
    if ((exec_pid = fork()) < 0)
    {
        printf("Error creating child process\n");
        exit(1);
    }
    if (exec_pid == 0)
    {
        execvp(args[0], args);
    }
    else
    {
        int status;
        waitpid(exec_pid, &status, 0);
    }
}
void list_hunts(int sig)
{
    char *args[] = {"./treasure_manager", "--list_hunts", NULL};
    fork_exec(args);
}
void list_treasures(int sig)
{
    int fd;
    char huntID[DEFAULT_LENGTH] = "";

    if ((fd = open("./commands.txt", O_RDONLY)) < 0)
    {
        perror("Error opening treasures file:add_treasure");
        exit(-1);
    }

    if (read(fd, huntID, DEFAULT_LENGTH) < 0)
    {
        perror("error reading");
    }
    if (close(fd) < 0)
    {
        perror("Error closing file");
    }

    char *args[] = {"./treasure_manager", "--list", huntID, NULL};
    fork_exec(args);
}
void view_treasure(int sig)
{
    int fd;
    char huntID[DEFAULT_LENGTH] = "";
    char treasureID[DEFAULT_LENGTH] = "";

    if ((fd = open("./commands.txt", O_RDONLY)) < 0)
    {
        perror("Error opening treasures file:add_treasure");
        exit(-1);
    }

    if (read(fd, huntID, DEFAULT_LENGTH) < 0)
    {
        perror("error reading");
    }
    if (read(fd, treasureID, DEFAULT_LENGTH) < 0)
    {
        perror("error reading");
    }
    if (close(fd) < 0)
    {
        perror("Error closing file");
    }

    char *args[] = {"./treasure_manager", "--view", huntID, treasureID, NULL};
    fork_exec(args);
}

void end_monitor_process(int sig)
{
    usleep(5000000);
    exit(EXIT_SUCCESS);
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

void monitor_ended(int sig)
{
    int status;
    pid_t ended_pid;

    while ((ended_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (ended_pid == monitor_pid)
        {
            printf("\n\nMonitor process has stopped");
            if (WIFEXITED(status))
            {
                printf(" with exit code %d.\n", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf(" due to signal %d.\n", WTERMSIG(status));
            }
            else
            {
                printf(" with an unknown status.\n");
            }

            // Reset monitor state variables
            monitor_running = 0;
            monitor_stopping = 0;
            monitor_pid = 0; // Important to reset PID
        }
    }

    // Check for errors with waitpid itself, though often ignored in simple handlers
    if (ended_pid == -1 && errno != ECHILD)
    {
        perror("waitpid error in monitor_ended");
    }
}

int main()
{

    struct sigaction parent_actions;
    memset(&parent_actions, 0x00, sizeof(struct sigaction));
    char huntID[DEFAULT_LENGTH] = "", treasureID[DEFAULT_LENGTH] = "";
    int fd;
    parent_actions.sa_handler = monitor_ended;
    if (sigaction(SIGCHLD, &parent_actions, NULL) < 0)
    {
        perror("Process a SIGCHLD sigaction");
        exit(-1);
    }

    while (1)
    {
        printf("Start monitor:sm\nList hunts:lh\nList treasures:lt\nView treasures:vt\nStop monitor:stm\nExit\nSelect a command:");
        char command[100] = "";
        scanf("%s", command);
        printf("\n");
        if (monitor_stopping == 1)
        {
            printf("Monitor procces is currently stopping. Can't accept any commands\n\n");
            continue;
        }
        if (strcmp(command, "sm") == 0)
        {
            if (monitor_running == 1)
            {
                printf("Monitor is curently running\n");
            }
            else
            {
                char *args[] = {"gcc", "-o", "treasure_manager", "./treasure_manager.c", NULL};
                fork_exec(args);
                if ((fd = open("./commands.txt", O_CREAT, mode)) < 0)
                {
                    perror("Error creating commands file");
                    exit(-1);
                }
                if (close(fd) == -1)
                {
                    perror("Error closing commands file");
                    exit(-1);
                }
                monitor_stopping = 0;
                monitor_running = 1;
                if ((monitor_pid = fork()) < 0)
                {
                    perror("Error creating child process\n");
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
        else if (strcmp(command, "lt") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing list_hunt command\n");
            }
            else
            {
                printf("HuntID:");
                scanf("%s", huntID);
                printf("\n");

                if ((fd = open("./commands.txt", O_WRONLY | O_TRUNC, mode)) == -1)
                {
                    perror("Error opening treasures file:add_treasure");
                    exit(-1);
                }
                if (write(fd, huntID, strlen(huntID)) < 0)
                {
                    perror("Error writing in file");
                }
                if (close(fd) < 0)
                {
                    perror("Error closing the file");
                }
                if (kill(monitor_pid, SIGUSR2) < 0)
                {
                    printf("Error sending SIGUSR to child\n");
                    exit(2);
                }
                sleep(1);
            }
        }
        else if (strcmp(command, "lh") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing list_treasures command\n");
            }
            else
            {

                if (kill(monitor_pid, SIGUSR1) < 0)
                {
                    printf("Error sending SIGUSR1 to child\n");
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
                printf("HuntID:");
                scanf("%s", huntID);
                printf("TreasureID:");
                scanf("%s", treasureID);

                if ((fd = open("./commands.txt", O_WRONLY | O_TRUNC, mode)) == -1)
                {
                    perror("Error opening treasures file:add_treasure");
                    exit(-1);
                }
                if (write(fd, huntID, DEFAULT_LENGTH) < 0)
                {
                    perror("Error writing in file");
                }
                if (write(fd, treasureID, DEFAULT_LENGTH) < 0)
                {
                    perror("Error writing in file");
                }
                if (close(fd) < 0)
                {
                    perror("Error closing the file");
                }

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
                printf("Monitor is not currently running\n");
            }
            else
            {
                int stop_pid;
                monitor_stopping = 1;
                if ((stop_pid = fork()) < 0)
                {
                    perror("");
                }
                else if (stop_pid == 0)
                {
                    if (kill(monitor_pid, SIGTERM) < 0)
                    {
                        perror("Error sending SIGTERM to monitor process");
                        // Exit child with error if kill fails
                        exit(2);
                    }
                    exit(0);
                }
                else
                {
                    printf("Monitor is stopping\n\n");
                    int status;
                    waitpid(stop_pid, &status, 0);
                    monitor_stopping = 1;
                    continue;
                }
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
        else if (strcmp(command, "") == 0)
        {
            continue;
        }
        else
        {

            printf("Invalid command---%s\n", command);
        }
        printf("\n");
    }
}