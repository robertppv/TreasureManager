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
    int fd;
    char path[1000] = "";
    char msjj[3000] = "";
    sprintf(path, "./Game/%s/loggedhunt", huntId);
    if ((fd = open(path, O_WRONLY | O_APPEND)) < 0)
    {
        perror("log");
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
        perror("log");
    }
    close(fd);
}
void print_treasure(treasure t)
{
    printf("Treasure ID:%s\nUsername:%s\nLatitude:%.2f\nLongitude:%.2f\nClue Text:%s\nValue:%d\n\n", t.treasureId, t.userName, t.coordinates.latitude, t.coordinates.longitude, t.clueText, t.Value);
}

void add_treasure(char *huntId)
{
    DIR *dirp = NULL;
    int fd;
    char path[2000] = "";
    char msj[2000] = "";
    sprintf(path, "./Game/%s", huntId);
    dirp = opendir(path);
    if (dirp == NULL)
    {
        mkdir(path, mode);
        int fdt;
        sprintf(path, "./Game/%s/loggedhunt", huntId);
        if ((fdt = open(path, O_CREAT, mode)) < 0)
        {
            perror("logfile");
            exit(-1);
        }
        sprintf(msj, "Hunt with id:%s was added.\n", huntId);
        add_to_log(huntId, msj);
        symlink();
        close(fdt);
    }
    else
    {
        closedir(dirp);
    }

    char treasureFile[1000] = "";
    sprintf(treasureFile, "%s_treasures.dat", huntId);
    sprintf(path, "./Game/%s/%s", huntId, treasureFile);

    fd = open(path, O_WRONLY | O_APPEND | O_CREAT, mode);
    if (fd == -1)
    {
        perror("eroare fisier");
    }

    treasure tr = getTreasureInfo();

    if (write(fd, &tr, sizeof(treasure)) == -1)
    {
        perror("eroare scriere");
    }
    sprintf(msj, "Treasure with id:%s was added.\n", tr.treasureId);
    add_to_log(huntId, msj);
    close(fd);
}

void remove_treasure(char *huntId, char *treasureId)
{
    char path[1000] = "";
    char patht[1000] = "";
    char msj[1000] = "";
    DIR *dir;
    int found = 0;
    sprintf(path, "./Game/%s", huntId);
    if ((dir = opendir(path)) == NULL)
    {
        perror("dir");
        exit(-1);
    }
    sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId);
    sprintf(patht, "./Game/%s/temp_treasures.dat", huntId);
    int fd, fdt;
    if ((fd = open(path, O_RDONLY, mode)) == -1)
    {
        perror("open:");
        exit(-1);
    }

    if ((fdt = open(patht, O_WRONLY | O_APPEND | O_CREAT, mode)) == -1)
    {
        perror("open temp");
        exit(-1);
    }
    treasure t;

    while (read(fd, &t, sizeof(treasure)) > 0)
    {
        if (strcmp(t.treasureId, treasureId) == 0)
        {
            found = 1;
        }
        else
        {
            write(fdt, &t, sizeof(treasure));
        }
    }

    if (found == 0)
    {
        unlink(patht);
        sprintf(msj, "Treasure with id:%s was not found in hunt with id:%s", treasureId, huntId);
    }
    else
    {
        unlink(path);
        rename(patht, path);
        sprintf(msj, "Treasure with id:%s was removed from hunt with id:%s", treasureId, huntId);
    }
    add_to_log(huntId, msj);
    close(fd);
    close(fdt);
}

void remove_hunt(char *huntId)
{

    char path[1000] = "";
    sprintf(path, "./Game/%s", huntId);
    char extcmd[2000] = "rm -rf ";
    sprintf(extcmd, "rm -rf %s/*", path);
    system(extcmd); // stergem directorul recursiv

    if (rmdir(path) == 0)
    {
        printf("Deleted\n");
    }
    else
    {
        printf("Hunt not found\n");
    }
}

void list(char *huntId)
{
    struct stat st;
    DIR *d;
    char path[1000] = "";
    sprintf(path, "./Game/%s", huntId);
    if ((d = opendir(path)) == NULL)
    {
        perror("Eroare director");
        exit(-1);
    }
    closedir(d);
    sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId);
    if (stat(path, &st) == -1)
    {
        perror("stat:");
        exit(-1);
    }
    printf("Hunt Id:%s\nFile Size:%ld bytes\nLast modification:%s\n", huntId, st.st_size, ctime(&st.st_mtime));
    int fd;
    if ((fd = open(path, O_RDONLY, mode)) < 0)
    {
        perror("file error");
        exit(-1);
    }
    treasure t;
    while (read(fd, &t, sizeof(treasure)) > 0)
    {
        print_treasure(t);
    }
    close(fd);
}

void view(char *huntId, char *treasureId)
{
    char path[1000] = "";
    sprintf(path, "./Game/%s", huntId);
    DIR *dir;
    dir = opendir(path);
    if (dir == NULL)
    {
        perror("directorul nu exista");
        exit(EXIT_FAILURE);
    }
    if (closedir(dir) == -1)
    {
        perror("Eroare director");
    }
    sprintf(path, "./Game/%s/%s_treasures.dat", huntId, huntId);
    int fd = open(path, O_RDONLY, mode);
    if (fd == -1)
    {
        perror("eroare fisier");
        exit(EXIT_FAILURE);
    }
    treasure tr;
    int found = 0;
    while (read(fd, &tr, sizeof(treasure)) > 0)
    {
        if (strcmp(tr.treasureId, treasureId) == 0)
        {
            print_treasure(tr);
            found = 1;
        }
    }
    if (close(fd) == -1)
    {
        perror("eroare fisier");
    }
    if (found == 0)
    {
        printf("Treasure cu id:%s nu exista.\n", treasureId);
    }
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        printf("Argumente insuficiente!\n");
        exit(-1);
    }
    mkdir("./Game",mode);
    if (strcmp(argv[1], "--add") == 0)
    {
        if (argc <= 2)
        {
            printf("Argumente insuficiente!\n");
        }
        else if (argc > 3)
        {
            printf("Prea multe argumente\n");
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
            printf("Prea multe argumente\n");
        }
        if (argc < 3)
        {
            printf("Prea putine argumente");
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
            printf("Argumente insuficiente");
        }
        else if (argc > 4)
        {
            printf("Prea multe argumente");
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
            printf("Prea multe argumente!\n");
            exit(-1);
        }
        else
        {
            remove_treasure(argv[2], argv[3]);
        }
    }
    else
    {
        printf("Comanda inexistenta\n");
    }

    return 0;
}