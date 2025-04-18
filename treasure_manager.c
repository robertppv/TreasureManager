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

#define PATH_LENGTH 512
#define MSJ_LENGTH 128
#define DEFAULT_LENGTH 32

mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;

typedef struct GPSCoordinates
{
    float latitude;
    float longitude;
} GPSCoordinates;

typedef struct treasure
{
    char treasureId[DEFAULT_LENGTH];
    char userName[DEFAULT_LENGTH];
    GPSCoordinates coordinates;
    char clueText[DEFAULT_LENGTH];
    int Value;
} treasure;

treasure getTreasureInfo()
{
    treasure t;
    memset(&t, 0, sizeof(treasure));

    printf("Treasure Id:");
    if (fgets(t.treasureId, DEFAULT_LENGTH, stdin) == NULL)
    {
        perror("Error reading Treasure Id");
        exit(-1);
    }

    if (t.treasureId[strlen(t.treasureId) - 1] != '\n')
    {
        printf("Input too long for Treasure Id.");
        exit(-1);
    }
    t.treasureId[strlen(t.treasureId) - 1] = '\0';

    printf("User name:");
    if (fgets(t.userName, DEFAULT_LENGTH, stdin) == NULL)
    {
        perror("Error reading User name");
        exit(-1);
    }
    if (t.userName[strlen(t.userName) - 1] != '\n')
    {
        printf("Input too long for User name.");
        exit(-1);
    }
    t.userName[strlen(t.userName) - 1] = '\0';

    char buffer[100];
    char *endptr;

    printf("Latitude:");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        perror("Error reading Latitude");
        exit(-1);
    }
    if (buffer[strlen(buffer) - 1] != '\n')
    {
        printf("Input too long for Latitude.\n");
        exit(-1);
    }
    buffer[strlen(buffer) - 1] = '\0';
    t.coordinates.latitude = strtof(buffer, &endptr);
    if (*endptr != '\0')
    {
        printf("Invalid input. Latitude must be a float.\n");
        exit(-1);
    }

    printf("Longitude:");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        perror("Error reading Longitude");
        exit(-1);
    }
    if (buffer[strlen(buffer) - 1] != '\n')
    {
        printf("Input too long for Longitude.\n");
        exit(-1);
    }
    buffer[strlen(buffer) - 1] = '\0';
    t.coordinates.longitude = strtof(buffer, &endptr);
    if (*endptr != '\0')
    {
        printf("Invalid input. Longitude must be a float.\n");
        exit(-1);
    }

    printf("Clue Text:");
    if (fgets(t.clueText, DEFAULT_LENGTH, stdin) == NULL)
    {
        perror("Error reading Clue Text");
        exit(-1);
    }
    if (t.clueText[strlen(t.clueText) - 1] != '\n')
    {
        printf("Input too long for Clue Text. Maximum length is %d.\n", DEFAULT_LENGTH - 1);
        exit(-1);
    }
    t.clueText[strlen(t.clueText) - 1] = '\0';

    printf("Value:");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        perror("Error reading Value");
        exit(-1);
    }
    if (buffer[strlen(buffer) - 1] != '\n')
    {
        printf("Input too long for Value.\n");
        exit(-1);
    }
    buffer[strlen(buffer) - 1] = '\0';
    t.Value = strtol(buffer, &endptr, 10);
    if (*endptr != '\0')
    {
        printf("Invalid input. Value must be an integer.\n");
        exit(-1);
    }

    return t;
}

void add_to_log(char *huntId, char *msj)
{
    if (msj == NULL)
    {
        exit(-1);
    }
    int fd;
    char path[PATH_LENGTH] = "", msjj[MSJ_LENGTH] = "";
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
int hunt_exists(char *huntId)
{
    char path[PATH_LENGTH] = "";
    DIR *dir;
    if (sprintf(path, "./Game/%s", huntId) < 0)
    {
        perror("Error making path:remove_treasure");
        exit(-1);
    }
    if ((dir = opendir(path)) == NULL)
    {
        return 0;
    }
    else
    {
        if (closedir(dir) == -1)
        {
            perror("Error closing dir:remove_treasure");
            exit(-1);
        }
        return 1;
    }
}
void add_treasure(char *huntId)
{
    int fd;
    char path[2 * PATH_LENGTH] = "", msj[MSJ_LENGTH] = "", lpath[PATH_LENGTH] = "", treasureFile[PATH_LENGTH] = "";
    treasure tr;
    tr = getTreasureInfo();

    if (hunt_exists(huntId) == 0)
    {
        if (sprintf(path, "./Game/%s", huntId) < 0)
        {
            perror("Error making path:remove_treasure");
            exit(-1);
        }
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
        if (sprintf(msj, "Hunt with id:%s was added.", huntId) < 0)
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
    if (sprintf(msj, "Treasure with id:%s was added.", tr.treasureId) < 0)
    {
        perror("Error making log message:add_treasure");
        exit(-1);
    }
    add_to_log(huntId, msj);
}
void remove_treasure(char *huntId, char *treasureId)
{
    char path[PATH_LENGTH] = "", msj[MSJ_LENGTH] = "";
    int fd, fdw, found = 0, pos = 0, res;
    treasure t;
    if (hunt_exists(huntId) == 0)
    {
        printf("Hunt with id:%s not found\n", huntId);
        exit(-1);
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
    char spath[PATH_LENGTH];
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

    char path[PATH_LENGTH] = "";

    if (hunt_exists(huntId) == 0)
    {
        printf("Hunt with id:%s not found\n", huntId);
    }
    else
    {
        if (sprintf(path, "./Game/%s", huntId) < 0)
        {
            perror("Error making path:list");
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
    char path[PATH_LENGTH] = "", msj[MSJ_LENGTH] = "";
    int fd, res = 0;
    treasure t;

    if (hunt_exists(huntId) == 0)
    {
        printf("Hunt with id:%s not found\n", huntId);
        exit(-1);
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
    if (sprintf(msj, "Hunt with id:%s was listed.", huntId) < 0)
    {
        perror("Error making log message:list");
        exit(-1);
    }
    add_to_log(huntId, msj);
}
void view(char *huntId, char *treasureId)
{
    char path[PATH_LENGTH] = "", msj[MSJ_LENGTH] = "";

    treasure tr;
    int found = 0, fd, res;

    if (hunt_exists(huntId) == 0)
    {
        printf("Hunt with id:%s not found", huntId);
        exit(-1);
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
            if (sprintf(msj, "Treasure with id:%s from hunt with id:%s was viewed.", treasureId, huntId) < 0)
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
{
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
}