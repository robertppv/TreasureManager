#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


int narg;
char **args;
mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;

typedef struct GPSCoordinates
{
    float latitude;
    float longitude;
};

typedef struct treasure
{
    char treasureId[100];
    char userName[20];
    struct GPSCoordinates coordinates;
    char clueText[100];
    int Value;
};

void add_treasure(char *huntId)
{
    
    DIR *dirp = NULL;
    char path[1000] = "./";
    strcat(path, huntId);
    dirp = opendir(path);
    if (dirp == NULL)
    {
        mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        dirp = opendir(path);
    }
    else
    {
        printf("Exista directorul");
        closedir(dirp);
    }
    // daca nu exista hunt creeam hunt cu id= huntId
}

void remove_treasure(char *huntId, char *treasureId)
{
}

void remove_hunt(char *huntId)
{

    char path[1000] = "./";
    char extcmd[1000] = "rm -rf ";
    strcat(path, huntId);
    strcat(extcmd,path);
    strcat(extcmd,"/*");
    system(extcmd);

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
    narg = argc;
    args = argv;
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
}