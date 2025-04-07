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
    char path[PATH_LENGTH] = "", msjj[2*PATH_LENGTH] = "";
    sprintf(path, "./Game/%s/loggedhunt", huntId);

    if ((fd = open(path, O_WRONLY | O_APPEND)) < 0)
    {
        perror("Error opening log file:add_to_log");
        exit(-1);
    }

    char buff[20];
    struct tm *t;
    time_t now = time(0);
    t = gmtime(&now);
    strftime(buff, sizeof(buff), "%d-%m-%Y %H:%M:%S", t);
    sprintf(msjj, "%s: %s", buff, msj);

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
    char path[2*PATH_LENGTH] = "", msj[PATH_LENGTH] = "", lpath[PATH_LENGTH] = "", treasureFile[PATH_LENGTH] = "";
    treasure tr;

    sprintf(path, "./Game/%s", huntId);
    if ((dirp = opendir(path)) == NULL)
    {
        mkdir(path, mode);
        sprintf(path, "./Game/%s/loggedhunt", huntId);
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
        sprintf(msj, "Hunt with id:%s was added.\n", huntId);
        add_to_log(huntId, msj);
        sprintf(lpath, "./loggedhunt-%s", huntId);
        symlink(path, lpath);
    }
    else
    {
        if (closedir(dirp) == -1)
        {
            perror("Error closing dir:add_treasure");
            exit(-1);
        }
    }

    sprintf(treasureFile, "%s_treasures.dat", huntId);
    sprintf(path, "./Game/%s/%s", huntId, treasureFile);

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
    sprintf(msj, "Treasure with id:%s was added.\n", tr.treasureId);
    add_to_log(huntId, msj);
}
void remove_treasure(char *huntId,char *treasureId)
{
    char path[2 * PATH_LENGTH] = "",msj[PATH_LENGTH]="";
    DIR *dir;
    int fd,fdw,found=0,pos=0;
    treasure t;
    sprintf(path, "./Game/%s", huntId);
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
    sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId);
    if((fd=open(path,O_RDWR))<0)
    {
         perror("Error opening treasures file:remove_treasure");
         exit(-1);
    }
    
    while(read(fd, &t, sizeof(treasure))>0)
    {
        if (strcmp(t.treasureId, treasureId) == 0)
        {
            found = 1;
            break;
        }else{
            pos++;
        }
    }
    
    if(found)
    {
        fdw = open(path, O_WRONLY);
        lseek(fdw, pos * sizeof(treasure), SEEK_SET);
        
        while (read(fd, &t, sizeof(treasure)) > 0)
        {
            if(write(fdw, &t, sizeof(treasure))<0)
            {
                perror("Error writing to treasures file");
            }
            pos++;
        }
        ftruncate(fdw, pos * sizeof(treasure));
        if(close(fdw)==-1)
        {
            perror("Error closing treasures file:remove_treasure");
            exit(-1);
        }
        sprintf(msj, "Treasure with id:%s was removed from hunt with id:%s", treasureId, huntId);
        add_to_log(huntId, msj);
    }
    else{
        printf("Treasure with id:%s from hunt with id:%s not found.\n", treasureId, huntId);
    }
    if(close(fd)==-1)
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
    

    if ((d = opendir(path)) == NULL)
    {
         perror("Error finding hunt:remove_rec");
        exit(-1);
    }

    while ((dp = readdir(d)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) 
            continue;
        sprintf(spath, "%s/%s", path, dp->d_name);

        if(dp->d_type== DT_DIR)
        {
            remove_rec(spath);
            unlink(spath);
        }
        else
        {
             if(dp->d_type== DT_REG)
             {
                 unlink(spath);
             }
             else{
                 printf("Invalid file");
                 exit(-1);
             }
        }
    }
    if (closedir(d) == -1)
        {
            perror("Error closing dir:remove_hunt2");
            exit(-1);
        }
    rmdir(path);
}
void remove_hunt(char *huntId)
{

    char path[1000] = "";
    sprintf(path, "./Game/%s", huntId);
    DIR *d;
    if ((d = opendir(path)) == NULL)
    {
         perror("Error finding hunt:remove_hunt");
    }
    else
    {
        remove_rec(path);
        sprintf(path, "./loggedhunt-%s", huntId);
        if(unlink(path)==-1)
        {
            perror("Error removing symlink");
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

    sprintf(path, "./Game/%s", huntId);
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

    sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId);
    if (stat(path, &st) == -1)
    {
        perror("Error using stat:list");
        exit(-1);
    }
    printf("Hunt Id:%s\nFile Size:%lld bytes\nLast modification:%s\n", huntId, st.st_size, ctime(&st.st_mtime));
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
    sprintf(msj, "Hunt with id:%s was listed.\n", huntId);
    add_to_log(huntId, msj);
}
void view(char *huntId, char *treasureId)
{
    char path[2*PATH_LENGTH] = "", msj[PATH_LENGTH] = "";
    DIR *dir;
    treasure tr;
    int found = 0, fd, res;

    sprintf(path, "./Game/%s", huntId);
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
    sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId);

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
            sprintf(msj, "Treasure with id:%s from hunt with id:%s was viewed.\n", treasureId, huntId);
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
    mkdir("./Game",mode);
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
    return 0;
}