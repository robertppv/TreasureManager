#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#define PATH_LENGTH 256

mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;

typedef struct GPSCoordinates
{
    float latitude;
    float longitude;
} GPSCoordinates;

typedef struct treasure
{
    char treasureId[20];
    char userName[20];
    GPSCoordinates coordinates;
    char clueText[20];
    int Value;
} treasure;

treasure getTreasureInfo()
{
    treasure t;
    memset(&t, 0, sizeof(treasure));

    printf("Treasure Id:");
    fgets(t.treasureId, 20, stdin);
    t.treasureId[strlen(t.treasureId) - 1] = 0;
    printf("User name:");
    fgets(t.userName, 20, stdin);
    t.userName[strlen(t.userName) - 1] = 0;
    printf("Latitude:");
    scanf("%f", &t.coordinates.latitude);
    printf("Longitude:");
    scanf("%f", &t.coordinates.longitude);
    getc(stdin);
    printf("Clue Text:");
    fgets(t.clueText, 20, stdin);
    t.clueText[strlen(t.clueText) - 1] = 0;
    printf("Value:");
    scanf("%d", &t.Value);
    return t;
}
void add_to_log(char *huntId, char *msj)
{
    if (msj == NULL)
    {
        exit(-1);
    }
    int fd;
    char path[PATH_LENGTH] = "", msjj[2 * PATH_LENGTH] = "";
    if (sprintf(path, "./Game/%s/loggedhunt", huntId) < 0)
    {
        perror("Error creating log file path:add_to_log");
        exit(-1);
    }

    if ((fd = open(path, O_WRONLY | O_APPEND)) < 0)
    {
        perror("Error opening log file:add_to_log");
        exit(-1);
    }

    char buff[20];
    struct tm *t;
    time_t now = time(0);
    if ((t = gmtime(&now)) == NULL)
    {
        perror("Error getting time:add_to_log");
        exit(-1);
    }
    if (strftime(buff, sizeof(buff), "%d-%m-%Y %H:%M:%S", t) == 0)
    {
        perror("Error formatting time:add_to_log");
        exit(-1);
    }
    if (sprintf(msjj, "%s: %s\n", buff, msj) < 0)
    {
        perror("Error formatting log message:add_to_log");
        exit(-1);
    }

    if (write(fd, msjj, strlen(msjj)) < 0)
    {
        perror("Error writing in log file:add_to_log");
        exit(-1);
    }
    if (close(fd) == -1)
    {
        perror("Error closing log file:add_to_log");
        exit(-1);
    }
}
void print_treasure(treasure t)
{
    printf("Treasure ID:%s\nUsername:%s\nLatitude:%.2f\nLongitude:%.2f\nClue Text:%s\nValue:%d\n\n", t.treasureId, t.userName, t.coordinates.latitude, t.coordinates.longitude, t.clueText, t.Value);
}
void add_treasure(char *huntId)
{
    DIR *dirp = NULL;
    int fd;
    char path[2 * PATH_LENGTH] = "", msj[PATH_LENGTH] = "", lpath[PATH_LENGTH] = "", treasureFile[PATH_LENGTH] = "";
    treasure tr;
    if (sprintf(path, "./Game/%s", huntId) < 0)
    {
        perror("Error creating directory path:add_treasure");
        exit(-1);
    }
    if ((dirp = opendir(path)) == NULL)
    {
        if (mkdir(path, mode) == -1)
        {
            perror("Error creating directory:add_treasure");
            exit(-1);
        }
        if (sprintf(path, "./Game/%s/loggedhunt", huntId) < 0)
        {
            perror("Error creating log file path:add_treasure");
            exit(-1);
        }
        if ((fd = open(path, O_CREAT, mode)) < 0)
        {
            perror("Error creating log file:add_treasure");
            exit(-1);
        }
        if (close(fd) == -1)
        {
            perror("Error closing log file:add_treasure");
            exit(-1);
        }
        if (sprintf(msj, "Hunt with id:%s was added.\n", huntId) < 0)
        {
            perror("Error making log message:add_treasure");
            exit(-1);
        }
        add_to_log(huntId, msj);
        if (sprintf(lpath, "./loggedhunt-%s", huntId) < 0)
        {
            perror("Error creating symlink path:add_treasure");
            exit(-1);
        }
        if (symlink(path, lpath) == -1)
        {
            perror("Error creating symlink:add_treasure");
            exit(-1);
        }
    }
    else
    {
        if (closedir(dirp) == -1)
        {
            perror("Error closing dir:add_treasure");
            exit(-1);
        }
    }

    if (sprintf(treasureFile, "%s_treasures.dat", huntId) < 0)
    {
        perror("Error creating treasure file name:add_treasure");
        exit(-1);
    }
    if (sprintf(path, "./Game/%s/%s", huntId, treasureFile) < 0)
    {
        perror("Error making treasure file path:add_treasure");
        exit(-1);
    }

    if ((fd = open(path, O_WRONLY | O_APPEND | O_CREAT, mode)) == -1)
    {
        perror("Error opening treasures file:add_treasure");
        exit(-1);
    }

    tr = getTreasureInfo();

    if (write(fd, &tr, sizeof(treasure)) == -1)
    {
        perror("Error writing in treasures file:add_treasure");
        exit(-1);
    }
    if (close(fd) == -1)
    {
        perror("Error closing treasures file:add_treasure");
        exit(-1);
    }
    if (sprintf(msj, "Treasure with id:%s was added.\n", tr.treasureId) < 0)
    {
        perror("Error making log message:add_treasure");
        exit(-1);
    }
    add_to_log(huntId, msj);
}
void remove_treasure(char *huntId, char *treasureId)
{
    char path[2 * PATH_LENGTH] = "", msj[PATH_LENGTH] = "";
    DIR *dir;
    int fd, fdw, found = 0, pos = 0, res;
    treasure t;
    if (sprintf(path, "./Game/%s", huntId) < 0)
    {
        perror("Error making path:remove_treasure");
        exit(-1);
    }
    if ((dir = opendir(path)) == NULL)
    {
        perror("Error finding hunt:remove_treasure");
        exit(-1);
    }
    else
    {
        if (closedir(dir) == -1)
        {
            perror("Error closing dir:remove_treasure");
            exit(-1);
        }
    }
    if (sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId) < 0)
    {
        perror("Error making treasure file path:view");
        exit(-1);
    }
    if ((fd = open(path, O_RDWR)) < 0)
    {
        perror("Error opening treasures file:remove_treasure");
        exit(-1);
    }

    while (read(fd, &t, sizeof(treasure)) > 0)
    {
        if (strcmp(t.treasureId, treasureId) == 0)
        {
            found = 1;
            break;
        }
        else
        {
            pos++;
        }
    }

    if (found)
    {
        if ((fdw = open(path, O_WRONLY)) < 0)
        {
            perror("Error opening treasures file for writing:remove_treasure");
            exit(-1);
        }
        if (lseek(fdw, pos * sizeof(treasure), SEEK_SET) == -1)
        {
            perror("Error seeking in treasures file:remove_treasure");
            exit(-1);
        }

        while ((res = read(fd, &t, sizeof(treasure))) > 0)
        {
            if (write(fdw, &t, sizeof(treasure)) < 0)
            {
                perror("Error writing to treasures file");
            }
            pos++;
        }
        if (res < 0)
        {
            perror("Error reading from file:remove_treasure");
            exit(-1);
        }
        if (ftruncate(fdw, pos * sizeof(treasure)) == -1)
        {
            perror("Error truncating treasures file:remove_treasure");
            exit(-1);
        }
        if (close(fdw) == -1)
        {
            perror("Error closing treasures file:remove_treasure");
            exit(-1);
        }
        if (sprintf(msj, "Treasure with id:%s was removed from hunt with id:%s", treasureId, huntId) < 0)
        {
            perror("Error making log message:remove_treasure");
            exit(-1);
        }
        add_to_log(huntId, msj);
    }
    else
    {
        printf("Treasure with id:%s from hunt with id:%s not found.\n", treasureId, huntId);
    }
    if (close(fd) == -1)
    {
        perror("Error closing treasures file:remove_treasure");
        exit(-1);
    }
}
void remove_rec(char *path)
{
    struct dirent *dp;
    char spath[5 * PATH_LENGTH];
    DIR *d;
    struct stat st;

    if ((d = opendir(path)) == NULL)
    {
        perror("Error finding hunt:remove_rec");
        exit(-1);
    }

    while ((dp = readdir(d)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        if (sprintf(spath, "%s/%s", path, dp->d_name) < 0)
        {
            perror("Error creating path:remove_rec");
            exit(-1);
        }
        if (stat(spath, &st) == -1)
        {
            perror("stat");
            exit(-1);
        }
        if (S_ISDIR(st.st_mode))
        {
            remove_rec(spath);
            if (unlink(spath) == -1)
            {
                perror("Error removing file:remove_rec");
                exit(-1);
            }
        }
        else
        {
            if (S_ISREG(st.st_mode))
            {
                if (unlink(spath) == -1)
                {
                    perror("Error removing file:remove_rec");
                    exit(-1);
                }
            }
            else
            {
                printf("Invalid file");
                exit(-1);
            }
        }
    }
    if (closedir(d) == -1)
    {
        perror("Error closing dir:remove_rec");
        exit(-1);
    }
    if (rmdir(path) == -1)
    {
        perror("Error removing dir:remove_rec");
        exit(-1);
    }
}
void remove_hunt(char *huntId)
{

    char path[2 * PATH_LENGTH] = "";
    DIR *d;
    if (sprintf(path, "./Game/%s", huntId) < 0)
    {
        perror("Error making path:list");
        exit(-1);
    }

    if ((d = opendir(path)) == NULL)
    {
        perror("Error finding hunt:remove_hunt");
    }
    else
    {
        if (closedir(d) == -1)
        {
            perror("Error closing dir:remove_hunt");
            exit(-1);
        }
        remove_rec(path);
        if (sprintf(path, "./loggedhunt-%s", huntId) < 0)
        {
            perror("Error making log symlink path");
            exit(-1);
        }
        if (unlink(path) == -1)
        {
            perror("Error removing symlink");
            exit(-1);
        }
    }
}
void list(char *huntId)
{
    struct stat st;
    DIR *d;
    char path[PATH_LENGTH] = "", msj[PATH_LENGTH] = "";
    int fd, res = 0;
    treasure t;

    if (sprintf(path, "./Game/%s", huntId) < 0)
    {
        perror("Error making path:list");
        exit(-1);
    }
    if ((d = opendir(path)) == NULL)
    {
        perror("Error finding hunt:list");
        exit(-1);
    }
    else
    {
        if (closedir(d) == -1)
        {
            perror("Error closing dir:list");
            exit(-1);
        }
    }

    if (sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId) < 0)
    {
        perror("Error making treasure file path:list");
        exit(-1);
    }
    if (stat(path, &st) == -1)
    {
        perror("Error using stat:list");
        exit(-1);
    }
    printf("Hunt Id:%s\nFile Size:%ld bytes\nLast modification:%s\n", huntId, st.st_size, ctime(&st.st_mtime));
    if ((fd = open(path, O_RDONLY, mode)) < 0)
    {
        perror("Error opening treasures file:list");
        exit(-1);
    }

    while ((res = read(fd, &t, sizeof(treasure))) > 0)
    {
        print_treasure(t);
    }

    if (res < 0)
    {
        perror("Error reading from treasures file:list");
        exit(-1);
    }

    if (close(fd) == -1)
    {
        perror("Error closing treasures file:list");
        exit(-1);
    }
    if (sprintf(msj, "Hunt with id:%s was listed.\n", huntId) < 0)
    {
        perror("Error making log message:list");
        exit(-1);
    }
    add_to_log(huntId, msj);
}
void view(char *huntId, char *treasureId)
{
    char path[2 * PATH_LENGTH] = "", msj[PATH_LENGTH] = "";
    DIR *dir;
    treasure tr;
    int found = 0, fd, res;

    if (sprintf(path, "./Game/%s", huntId) < 0)
    {
        perror("Error making path:view");
        exit(-1);
    }
    if ((dir = opendir(path)) == NULL)
    {
        perror("Error finding hunt:view");
        exit(-1);
    }
    else
    {
        if (closedir(dir) == -1)
        {
            perror("Error closing dir:view");
            exit(-1);
        }
    }
    if (sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId) < 0)
    {
        perror("Error making treasure file path:view");
        exit(-1);
    }

    if ((fd = open(path, O_RDONLY, mode)) == -1)
    {
        perror("Error opening treasures file:view");
        exit(-1);
    }

    while ((res = read(fd, &tr, sizeof(treasure))) > 0)
    {
        if (strcmp(tr.treasureId, treasureId) == 0)
        {
            print_treasure(tr);
            found = 1;
            if (sprintf(msj, "Treasure with id:%s from hunt with id:%s was viewed.\n", treasureId, huntId) < 0)
            {
                perror("Error making log message:view");
                exit(-1);
            }
            add_to_log(huntId, msj);
            break;
        }
    }
    if (res < 0)
    {
        perror("Error reading from treasures file:view");
        exit(-1);
    }
    if (close(fd) == -1)
    {
        perror("Error closing treasures file:view");
        exit(-1);
    }
    if (found == 0)
    {
        printf("Treasure with id:%s not found.\n", treasureId);
    }
}

int main(int argc, char **argv)
{ /*
     if (argc == 1)
     {
         printf("Not enough arguments!\n");
         exit(-1);
     }
     mkdir("./Game", mode);
     if (strcmp(argv[1], "--add") == 0)
     {
         if (argc <= 2)
         {
             printf("Not enough arguments!\n");
         }
         else if (argc > 3)
         {
             printf("To many arguments!\n");
         }
         else
         {
             add_treasure(argv[2]);
         }
     }
     else if (strcmp(argv[1], "--list") == 0)
     {
         if (argc > 3)
         {
             printf("To many arguments!\n");
         }
         else if (argc < 3)
         {
             printf("Not enough arguments!\n");
         }
         else
         {
             list(argv[2]);
         }
     }
     else if (strcmp(argv[1], "--view") == 0)
     {
         if (argc < 4)
         {
             printf("Not enough arguments!\n");
         }
         else if (argc > 4)
         {
             printf("To many arguments!\n");
         }
         else
         {
             view(argv[2], argv[3]);
         }
     }
     else if (strcmp(argv[1], "--remove") == 0)
     {
         if (argc == 3)
         {
             remove_hunt(argv[2]);
         }
         else if (argc > 4)
         {
             printf("To many arguments!\n");
             exit(-1);
         }
         else
         {
             remove_treasure(argv[2], argv[3]);
         }
     }
     else
     {
         printf("Unknown command\n");
     }
         */
    char buffer[100];
    double value;
    char *endptr;
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return -1; /* Unexpected error */
    value = strtod(buffer, &endptr);
    if ((*endptr == '\0') || (isspace(*endptr) != 0))
        printf("It's float: %f\n", value);
    else
        printf("It's NOT float ...\n");
    return 0;
}