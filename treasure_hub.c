#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
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
#include <sys/stat.h>
#include <dirent.h>
int monitor_running = 0;
int monitor_stopping = 0;
int pipefd[2];
pid_t monitor_pid;
mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
#define DEFAULT_LENGTH 32

void fork_exec(char **args)
{
    int exec_pid;
    if ((exec_pid = fork()) < 0)
    {
        printf("Error creating child process\n");
        exit(EXIT_FAILURE);
    }
    if (exec_pid == 0)
    {
        execvp(args[0], args);
        perror("execvp failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        waitpid(exec_pid, &status, 0);
    }
}
void list_hunts()
{
    char *args[] = {"./treasure_manager", "--list_hunts", NULL};
    fork_exec(args);
}
void list_treasures(char *huntID)
{
    char *args[] = {"./treasure_manager", "--list", huntID, NULL};
    fork_exec(args);
}
void view_treasure(char *huntID, char *treasureID)
{
    char *args[] = {"./treasure_manager", "--view", huntID, treasureID, NULL};
    fork_exec(args);
}
void end_monitor_process(int sig)
{
    usleep(5000000);
    exit(EXIT_SUCCESS);
}
void handle_commands(int sig)
{
    char command[2], huntID[DEFAULT_LENGTH] = "", treasureID[DEFAULT_LENGTH] = "";
    int fd;

    if ((fd = open("./commands.txt", O_RDONLY)) < 0)
    {
        perror("Error opening commands file");
        exit(EXIT_FAILURE);
    }
    if (read(fd, command, 2) < 0)
    {
        perror("error reading");
        exit(EXIT_FAILURE);
    }
    if (strcmp(command, "1") == 0)
    {
        if (read(fd, huntID, DEFAULT_LENGTH) < 0)
        {
            perror("error reading");
            exit(-1);
        }

        list_treasures(huntID);
    }
    else if (strcmp(command, "3") == 0)
    {
        if (read(fd, huntID, DEFAULT_LENGTH) < 0)
        {
            perror("error reading");
            exit(-1);
        }
        if (read(fd, treasureID, DEFAULT_LENGTH) < 0)
        {
            perror("error reading");
            exit(-1);
        }
        view_treasure(huntID, treasureID);
    }
    else if (strcmp(command, "2") == 0)
    {
        list_hunts();
    }
    else
    {
        printf("Invalid command\n");
    }
    if (close(fd) < 0)
    {
        perror("Error closing file");
    }
}
void monitor_procces()
{
    struct sigaction monitor_actions;
    memset(&monitor_actions, 0x00, sizeof(struct sigaction));
    printf("Monitor procces started\n");
    dup2(pipefd[1], STDOUT_FILENO);
    monitor_actions.sa_handler = handle_commands;
    if (sigaction(SIGUSR1, &monitor_actions, NULL) < 0)
    {
        perror("Process a SIGUSR1 sigaction");
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
                printf(" with exit code %d.", WEXITSTATUS(status));
            }
            else if (WIFSIGNALED(status))
            {
                printf(" due to signal %d.\n", WTERMSIG(status));
            }
            else
            {
                printf(" with an unknown status.\n");
            }

            monitor_running = 0;
            monitor_stopping = 0;
            monitor_pid = 0;
        }
    }

    if (ended_pid == -1 && errno != ECHILD)
    {
        perror("waitpid error in monitor_ended");
        exit(-1);
    }
}

void read_pipe(int pipefd)
{
    char buff[256];
    ssize_t bytes_read;

    while (1)
    {
        bytes_read = read(pipefd, buff, sizeof(buff) - 1);
        if (bytes_read > 0)
        {
            buff[bytes_read] = '\0';
            printf("%s", buff);
        }
        else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        {
            break;
        }
        else if (bytes_read == 0)
        {
            break;
        }
        else
        {
            perror("Error reading from pipe");
            break;
        }
    }
}

void calculate_score()
{
    struct dirent *dp;
    DIR *d;
    int pid;
    int ok = 0;

    if ((d = opendir("./Game")) == NULL)
    {
        perror("Error opening game directory:");
        exit(-1);
    }
    char *args[] = {"gcc", "-o", "calculate_score", "./calculate_score.c", NULL};
    fork_exec(args);
    while ((dp = readdir(d)) != NULL)
    {

        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        int pipec_s[2];
        if (pipe(pipec_s) < 0)
        {
            perror("pipe error");
            exit(-1);
        }
        if ((pid = fork()) < 0)
        {
            perror("err");
            exit(-1);
        }
        else if (pid == 0)
        {
            if (close(pipec_s[0]) < 0)
            {
                perror("Error closing pipe fd");
                exit(-1);
            }
            dup2(pipec_s[1], STDOUT_FILENO);

            char *args2[] = {"./calculate_score", dp->d_name, NULL};
            fork_exec(args2);

            if (close(pipec_s[1]) < 0)
            {
                perror("Error closing pipe fd");
                exit(-1);
            }
            exit(0);
        }
        else
        {
            if (close(pipec_s[1]) < 0)
            {
                perror("Error closing pipe fd");
                exit(-1);
            }
            ok = 1;
            int status;
            waitpid(pid, &status, 0);
            read_pipe(pipec_s[0]);
        }
    }

    if (ok == 0)
    {
        printf("No hunts\n");
    }

    if (closedir(d) == -1)
    {
        perror("Error closing dir");
        exit(-1);
    }
    sleep(1);
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
    mkdir("Game", mode);
    while (1)
    {
        printf("start_monitor\nlist_hunts\nlist_treasures\nview_treasure\ncalculate_score\nstop_monitor\nexit\nSelect a command:");
        char command[100] = "";
        int res = scanf("%s", command);
        if (res == EOF)
        {
            clearerr(stdin);
            printf("\n\n");
            continue;
        }
        printf("\n");
        if (monitor_stopping == 1)
        {
            printf("Monitor procces is currently stopping. Can't accept any commands\n\n");
            continue;
        }
        if (strcmp(command, "start_monitor") == 0)
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
                if (close(fd) < 0)
                {
                    perror("Error closing commands file");
                    exit(-1);
                }
                monitor_stopping = 0;
                monitor_running = 1;
                if (pipe(pipefd) < 0)
                {
                    perror("pipe error");
                    exit(-1);
                }

                if ((monitor_pid = fork()) < 0)
                {
                    perror("Error creating child process\n");
                    exit(-1);
                }
                if (monitor_pid == 0)
                {
                    close(pipefd[0]);
                    monitor_procces();
                    exit(0);
                }
                else
                {
                    close(pipefd[1]);
                    if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) < 0)
                        exit(2);
                    sleep(1);
                    printf("\n");
                    continue;
                }
            }
        }
        else if (strcmp(command, "list_treasures") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing list_treasures command\n");
            }
            else
            {
                printf("HuntID:");
                scanf("%s", huntID);
                printf("\n");

                if ((fd = open("./commands.txt", O_WRONLY | O_TRUNC, mode)) == -1)
                {
                    perror("Error opening commnads file");
                    exit(-1);
                }
                if (write(fd, "1", 2) < 0)
                {
                    perror("Error writing in file");
                }
                if (write(fd, huntID, strlen(huntID)) < 0)
                {
                    perror("Error writing in file");
                }
                if (close(fd) < 0)
                {
                    perror("Error closing the file");
                }
                if (kill(monitor_pid, SIGUSR1) < 0)
                {
                    printf("Error sending SIGUSR to child\n");
                    exit(2);
                }
                sleep(1);
                read_pipe(pipefd[0]);
                sleep(1);
            }
        }
        else if (strcmp(command, "list_hunts") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Start monitor before executing list_hunts command\n");
            }
            else
            {
                if ((fd = open("./commands.txt", O_WRONLY | O_TRUNC, mode)) == -1)
                {
                    perror("Error opening treasures file:add_treasure");
                    exit(-1);
                }
                if (write(fd, "2", 2) < 0)
                {
                    perror("Error writing in file");
                }
                if (close(fd) < 0)
                {
                    perror("Error closing the file");
                }
                if (kill(monitor_pid, SIGUSR1) < 0)
                {
                    printf("Error sending SIGUSR1 to child\n");
                    exit(2);
                }
                sleep(1);
                read_pipe(pipefd[0]);
                sleep(1);
            }
        }
        else if (strcmp(command, "view_treasure") == 0)
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
                if (write(fd, "3", 2) < 0)
                {
                    perror("Error writing in file");
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

                if (kill(monitor_pid, SIGUSR1) < 0)
                {
                    printf("Error sending SIGILL to child\n");
                    exit(2);
                }
                sleep(1);
                read_pipe(pipefd[0]);
                sleep(1);
            }
        }
        else if (strcmp(command, "stop_monitor") == 0)
        {
            if (monitor_running == 0)
            {
                printf("Monitor is not currently running\n");
            }
            else
            {
                if (close(pipefd[0]) < 0)
                {
                    perror("Error closing pipe");
                }
                int stop_pid;
                monitor_stopping = 1;

                if ((stop_pid = fork()) < 0)
                {
                    perror("fork");
                }
                else if (stop_pid == 0)
                {
                    if (kill(monitor_pid, SIGTERM) < 0)
                    {
                        perror("Error sending SIGTERM to monitor process");

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
        else if (strcmp(command, "calculate_score") == 0)
        {

            calculate_score();
        }
        else if (strcmp(command, "") == 0)
        {
            continue;
        }
        else
        {
            printf("Invalid command\n");
        }
        printf("\n");
    }
}