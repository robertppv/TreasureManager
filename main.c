#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>



mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;

typedef struct GPSCoordinates
{
    float latitude;
    float longitude;
}GPSCoordinates;

typedef struct treasure
{
    char treasureId[20];
    char userName[20];
    //GPSCoordinates coordinates;
    char clueText[50];
    int Value;
}treasure;
treasure getTreasureInfo()
{
    treasure t;

    printf("Treasure Id:");
    scanf("%s", t.treasureId);
    printf("User name:");
    scanf("%s", t.userName);
    
    // printf("Latitude:");
    // scanf("%f", &t.coordinates.latitude);
    // printf("longitude:");
    // scanf("%f", &t.coordinates.longitude);
    printf("Clue Text:");
    scanf("%s", t.clueText);
    printf("Value:");
    scanf("%d", &t.Value);
    return t;
}
void add_treasure(char *huntId)
{
    
    DIR *dirp = NULL;
    int fd;
    char path[2000] = "";
    sprintf(path, "./%s", huntId);
    dirp = opendir(path);
    if (dirp == NULL)
    {
        mkdir(path,mode);
        
    }
    else
    {
        printf("Exista directorul");
        closedir(dirp);
    }
     
    char treasureFile[1000] = "";
    sprintf(treasureFile, "%s_treasures.dat", huntId);
    sprintf(path,"./%s/%s", huntId, treasureFile);
    
    fd = open(path, O_RDWR | O_APPEND | O_CREAT , mode);
    
    treasure tr = getTreasureInfo();

    write(fd, &tr, sizeof(treasure));
    treasure t2;
    int res=read(fd, &t2, sizeof(treasure));
    
    printf("=====%d===%d===", res, t2.Value);
    close(fd);
}


void remove_treasure(char *huntId, char *treasureId)
{
}

void remove_hunt(char *huntId)
{

    char path[1000] = "";
    sprintf(path, "./%s", huntId);
    char extcmd[2000] = "rm -rf ";
    sprintf(extcmd, "rm -rf %s/*", path);
    system(extcmd); // stergem directorul recursiv

    if(rmdir(path)==0)
    {
        printf("directorul a fost sters");
    }else
        printf("directorul nu exista");
}

void list(char *huntId)
{
}

void view(char *huntId, char *treasureId)
{
}




int main(int argc, char **argv)
{
    if (argc == 1)
    {
        printf("Argumente insuficiente!");
        exit(-1);
    }
    
    if (strcmp(argv[1], "--add") == 0)
    {
        if(argc ==1)
        {
            printf("Argumente insuficiente!");
        }
        else if (argc > 3)
        {
            printf("Prea multe argumente");
        }
        add_treasure(argv[2]);
    }
    else if (strcmp(argv[1], "--list") == 0)
    {
    }
    else if (strcmp(argv[1], "--view") == 0)
    {
    }
    else if (strcmp(argv[1], "--remove") == 0)
    {
        if (argc == 3)
        {
            remove_hunt(argv[2]);
        }
        else if (argc > 4)
        {
            printf("Prea multe argumente!");
            exit(-1);
        }

        //remove treasure from hunt
    }
    else
    {
        printf("Comanda inexistenta");
    }
    return 0;
}